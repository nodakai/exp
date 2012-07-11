#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <utility>

namespace receive_quote {

using namespace std;

class Symbol {
private:
    string m_sym;
public:
    Symbol(const string &sym) : m_sym(sym) { }
    string str() const { return m_sym; }

    bool operator< (const Symbol &o) const { return m_sym < o.m_sym; }
};

class Request {
private:
    Symbol m_symbol;
    int m_begin;
    int m_end;

public:
    Request(const Symbol &symbol, int begin, int end) : m_symbol(symbol), m_begin(begin), m_end(end) { }
    const Symbol &symbol() const { return m_symbol; }
    int begin() const { return m_begin; }
    int end() const { return m_end; }
};

typedef int Price;

class ResponseBody {
private:
    Price m_price;
public:
    ResponseBody(Price price) : m_price(price) { }
    Price price() const { return m_price; }
};

class Response {
private:
    Request m_req;
    ResponseBody m_body;
    int m_total;
    int m_seq;
public:
    Response(const Request &req, const ResponseBody &body, int total, int seq)
        : m_req(req), m_body(body), m_total(total), m_seq(seq)
    { }

    const Request &request() const { return m_req; }
    const ResponseBody &body() const { return m_body; }
    int total() const { return m_total; }
    int sequence() const { return m_seq; }
};

typedef vector<Price> VPrices;

class OutElem {
private:
    Symbol m_symbol;
    VPrices m_prices;

public:
    OutElem(const Symbol &symbol) : m_symbol(symbol)
    { m_prices.reserve(1000); }

    void add(Price p) {
        m_prices.push_back(p);
    }

    string str() const {
        ostringstream oss;
        oss << m_symbol.str() << " ";
        for (VPrices::const_iterator it(m_prices.begin()), itEnd(m_prices.end()); it != itEnd; ++it) {
            oss << *it << ";" ;
        }
        return oss.str();
    }
};

typedef vector<Response> VResp;
typedef vector<OutElem> VOut;

typedef int Seq;

typedef pair<Symbol, Seq> Key;
struct Value {
public:
    int total;
    VPrices ps;
};
typedef map<Key, Value> Map;

VOut receive_quote(const VResp &resps)
{
    VOut res;

    Map map;
    for (VResp::const_iterator it(resps.begin()), itEnd(resps.end()); it != itEnd; ++it) {
        const Key k = make_pair(it->request().symbol(), it->sequence()-1);
        Value &v = map[];
        if (0 == it->sequence()) {
            v.total = it->total();
        }
        v.ps.push_back(it->body().price());
        ps.
        if (v.total == it->sequence() - 1) {
        }
    }
    return res;
}

} // namespace receive_quote

int main(int argc, char *argv[])
{
    using namespace receive_quote;

    VResp resps;

    cout << "Ready..." ;
    const Request a(Symbol("A"), 1, 4), b(Symbol("B"), 1, 2);
    resps.push_back(Response(a, ResponseBody(1100), 4, 0));
    resps.push_back(Response(a, ResponseBody(1098), 4, 1));
    resps.push_back(Response(a, ResponseBody(1100), 4, 0));
    resps.push_back(Response(b, ResponseBody( 910), 2, 0));
    resps.push_back(Response(a, ResponseBody(1098), 4, 1));
    resps.push_back(Response(a, ResponseBody(1081), 4, 2));
    resps.push_back(Response(a, ResponseBody(1081), 4, 2));
    resps.push_back(Response(a, ResponseBody(1120), 4, 3));
    resps.push_back(Response(b, ResponseBody( 940), 2, 1));
    resps.push_back(Response(a, ResponseBody(1120), 4, 3));
    cout << endl;

    cout << "Go! ... " ;
    const VOut &output = receive_quote::receive_quote(resps);
    cout << "Done." << endl;
    for (VOut::const_iterator it(output.begin()), itEnd(output.end()); it != itEnd; ++it) {
        cout << it->str() << endl;
    }
}
