#include <iostream>
#include <utility>
#include "random"
#include "set"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <math.h>
#include <clocale>

//#include <boost/random/mersenne_twister.hpp>

/* initialize state to random bits */
static unsigned long state[16];
/* init should also reset this to 0 */
static unsigned int index2 = 0;

/* return 32 bit random number */
unsigned long WELLRNG512(void) {
    unsigned long a, b, c, d;
    a = state[index2];
    c = state[(index2 + 13) & 15];
    b = a ^ c ^ (a << 16) ^ (c << 15);
    c = state[(index2 + 9) & 15];
    c ^= (c >> 11);
    a = state[index2] = b ^ c;
    d = a ^ ((a << 5) & 0xDA442D24UL);
    index2 = (index2 + 15) & 15;
    a = state[index2];
    state[index2] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
    return state[index2];
}

using namespace std;
using namespace std::chrono;
typedef mersenne_twister_engine<uint32_t, 32, 351, 175, 19, 0xccab8ee7, 11, 0xffffffff, 7, 0x31b6ab00, 15, 0xffe50000, 17, 1812433253> mt11213b;

int compareStrings(string s1, string s2, int lId, int rId) {
    int sqrtN = (int) sqrt(rId - lId + 1.0);
    int r = -1;
    for (int i = 0; i < sqrtN; i++) {
        r = WELLRNG512() % (rId - lId + 1) + lId;
        if (s1[r] == s2[r]) {
            return r;
        }
    }
    return r;
}

int compareStrings(string s1, string s2) {
    int maxLength = min(s1.size(), s2.size());
    int firstUncommonCharIndex = compareStrings(s1, s2, 0, maxLength);

    // Строки совпали
    if (firstUncommonCharIndex == -1) {
        return 0;
    }
    return s1[firstUncommonCharIndex] > s2[firstUncommonCharIndex] ?
           1 :
           -1;
}

struct Response {
    Response(Response *pResponse) {
    }

    string word;
    long weight;

    Response(string w, long we) : word(w), weight(we) {}

    const bool operator=(const Response &obj) { return word == obj.word; }

    bool operator<(const Response &obj) const { return (this->weight < obj.weight); }

};

set<Response> mergeMaxResponses(set<Response> maxResponseLeft, set<Response> maxResponseRight) {
    set < Response > res, ret;
    std::set_union(maxResponseLeft.begin(), maxResponseLeft.end(),
                   maxResponseRight.begin(), maxResponseRight.end(),
                   inserter(res, std::begin(res)));
    auto x = res.begin();
    for (int i = 0; i < 5 && x != res.end(); ++i, ++x) {
        ret.insert(*x);
    }
    return ret;
}


struct Node {
    string key;
    int prior;
    int val, size;
    Node *l = 0, *r = 0;
    set<Response> max = set < Response > ();

    Node(string key, int val);

    Node(Node *t);
};

Node::Node(string k, int v) {
    key = k;
    prior = random();
    val = v;
    size = 1;
    max.insert(Response(k, v));
}


int size(Node *v) { return v ? v->size : 0; }

set<Response> max(Node *v) { return v ? v->max : (set < Response > ()); }

set<Response> mergeForNode(Node *v) {
    set < Response > res, ret;
    res.insert(Response(v->key, v->val));
    res = mergeMaxResponses(res, max(v->l));
    res = mergeMaxResponses(res, max(v->r));
    auto x = res.begin();
    for (int i = 0; i < 5 && x != res.end(); ++i, ++x) {
        ret.insert(*x);
    }
    return ret;
}

void upd(Node *v) {
    v->size = 1 + size(v->l) + size(v->r);
    v->max = mergeForNode(v);
}

Node::Node(Node *t) {
    if (t == 0) return;
    key = t->key;
    prior = t->prior;
    val = t->val;
    size = t->size;
    l = t->l;
    r = t->r;
    max = t->max;
}

Node *merge(Node *l, Node *r) {
    if (!l) return r;
    if (!r) return l;
    l = new Node(l), r = new Node(r);
    if (rand() % (size(l) + size(r)) < size(l)) {
        l->r = merge(l->r, r);
        upd(l);
        return l;
    } else {
        r->l = merge(l, r->l);
        upd(r);
        return r;
    }
}

pair<Node *, Node *> split(Node *p, string x) {
    if (!p) return {0, 0};
    p = new Node(p);
    if (compareStrings(p->key, x) <= 0) {
        auto [l, r] = split(p->r, x);
        p->r = l;
        upd(p);
        return {p, r};
    } else {
        auto [l, r] = split(p->l, x);
        p->l = r;
        upd(p);
        return {l, p};
    }
}

Node *insert(string x, int w, Node *root) {
    auto [l, r] = split(root, x);
    Node *t = new Node(x, w);
    return merge(l, merge(t, r));
}

set<Response> solve(Node *root, const string &search);

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    setlocale(LC_ALL, "ru_RU.UTF-8");
    std::ifstream myfile;
    myfile.open("/Users/musin/CLionProjects/vkr/1000/russianWithRang.txt");
    Node *root = nullptr;
    string line;
    string key, wS;
    while (myfile) {

        getline(myfile, line);
        stringstream ssin(line);
        ssin >> key;
        ssin >> wS;
        root = insert(key, stoi(wS), root);
    }
    ofstream myOfile;
    myOfile.open("/Users/musin/CLionProjects/vkr/1000/result.txt");
    std::ifstream myfileQ;
    cout << "started" << endl;

    myfileQ.open("/Users/musin/CLionProjects/vkr/1000/words.txt");
    string Q, Ans;
    while (myfileQ) {

        getline(myfileQ, line);
        if (line == "")
            break;
        stringstream ssin(line);
        ssin >> Q;
        ssin >> Ans;

        auto start = std::chrono::high_resolution_clock::now();
        auto res = solve(root, Q);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        auto durationNanos = duration_cast<microseconds>(end - start);

        set < string > responses;
        for (const auto &item: res) {
            responses.insert(item.word);
        }

        myOfile << Q << " ";
        myOfile << Q.length() << " ";
        myOfile << durationNanos.count() << " ";
        myOfile << responses.contains(Q) << endl;

        cout << Q << " : ";
        for (const auto &item: responses) {
            cout << item << " ";
        }
        cout << endl;

    }

    myOfile.close();

    return 0;
}


set<Response> solve(Node *root, const string &search) {
    vector<pair<Node *, bool>> roots;
    roots.emplace_back(root, false);

    string curSearch = "";
    for (int j = 0; j < search.size(); j +=2) {
        int c = search[j]+ search[j+1];
        curSearch += to_string(c);
        Node *newNode = nullptr;
        Node *toDelete = nullptr;
        int N = roots.size();
        for (int i = 0; i < N; i++) {
            auto curR = roots[i];
            // если мы уже учли опечатку - двигаем правую границу для текущего множества
            if (size(curR.first) == 1) {
                if (curR.second) {
                    if (curR.first->key.length() < curSearch.length() - 1 ||
                        curR.first->key[curSearch.length() - 1] != c) {

                        toDelete = curR.first;
                        continue;
                    }
                }
            }
            if (curR.second) {
                string sforSplit = curSearch;
                sforSplit[sforSplit.length() - 1] = char(sforSplit[sforSplit.length() - 1] + 1);
                auto [l, r] = split(curR.first, sforSplit);
                curR.first = l;
                continue;
            }
            // если не учли - 2 варианта
            // говорим что ошибка в нашем символе  - добавляем в список новое множество
            roots.push_back(make_pair(curR.first, true));

            // говорим что ошибок не было - двигаем левую границу для текущего множества
            auto [l, r] = split(curR.first, curSearch);
            curR.first = r;
        }
        if (toDelete != 0) {
            roots.erase(std::find(roots.begin(), roots.end(), make_pair(toDelete, false)));
        }
    }
    set < Response > res;

    for (auto r: roots) {
        res = mergeMaxResponses(res, r.first->max);
    }
    return res;
}



