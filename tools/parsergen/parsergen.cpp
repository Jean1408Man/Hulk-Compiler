#include <algorithm>
#include <cctype>
#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

enum class Assoc {
    None,
    Left,
    Right,
    Nonassoc,
};

struct SymbolSpec {
    std::string name;
    bool terminal = false;
    std::string type;
    int id = -1;
    int precedence = 0;
    Assoc assoc = Assoc::None;
};

struct Production {
    int id = -1;
    std::string lhs_name;
    int lhs = -1;
    std::vector<std::string> rhs_names;
    std::vector<int> rhs;
    std::string action;
    std::string explicit_prec;
    int precedence = 0;
    Assoc assoc = Assoc::None;
};

struct Grammar {
    std::string start;
    std::vector<SymbolSpec> terminals;
    std::vector<SymbolSpec> nonterminals;
    std::unordered_map<std::string, int> terminal_by_name;
    std::unordered_map<std::string, int> nonterminal_by_name;
    std::unordered_map<int, std::string> type_by_symbol;
    std::vector<Production> productions; // Actual grammar rules, numbered from 1.
    int end_symbol = -1;
    int accept_symbol = -1;

    int terminal_count() const {
        return static_cast<int>(terminals.size());
    }

    int nonterminal_count() const {
        return static_cast<int>(nonterminals.size());
    }

    int symbol_count() const {
        return terminal_count() + nonterminal_count() + 1;
    }
};

struct Item {
    int production = -1;
    int dot = 0;
    int lookahead = -1;

    bool operator<(const Item& other) const {
        if (production != other.production) return production < other.production;
        if (dot != other.dot) return dot < other.dot;
        return lookahead < other.lookahead;
    }
};

struct CoreItem {
    int production = -1;
    int dot = 0;

    bool operator<(const CoreItem& other) const {
        if (production != other.production) return production < other.production;
        return dot < other.dot;
    }
};

enum class ActionKind {
    Error,
    Shift,
    Reduce,
    Accept,
};

struct TableAction {
    ActionKind kind = ActionKind::Error;
    int target = -1;
    int rule = -1;
    int shift_rule = -1;
};

struct Conflict {
    std::string kind;
    std::string token;
    int state = -1;
    int reduce_rule = -1;
    int shift_rule = -1;
    int other_rule = -1;

    std::string normalized() const {
        std::ostringstream out;
        if (kind == "sr") {
            out << "sr token=" << token
                << " reduce_rule=" << reduce_rule
                << " shift_rule=" << shift_rule;
        } else {
            out << "rr token=" << token
                << " keep_rule=" << reduce_rule
                << " drop_rule=" << other_rule;
        }
        return out.str();
    }
};

struct Tables {
    std::vector<std::vector<TableAction>> action;
    std::vector<std::vector<int>> go_to;
    std::vector<Conflict> conflicts;
};

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open " + path);
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("cannot write " + path);
    }
    out << content;
}

std::string trim(const std::string& text) {
    std::size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }
    std::size_t last = text.size();
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }
    return text.substr(first, last - first);
}

bool starts_with(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

bool is_number_token(const std::string& token) {
    return !token.empty() &&
           std::all_of(token.begin(), token.end(), [](unsigned char c) {
               return std::isdigit(c) != 0;
           });
}

std::vector<std::string> split_words(const std::string& line) {
    std::vector<std::string> words;
    std::string current;
    bool in_angle = false;
    for (char c : line) {
        if (c == '<') {
            in_angle = true;
            current += c;
        } else if (c == '>') {
            in_angle = false;
            current += c;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !in_angle) {
            if (!current.empty()) {
                words.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

std::string strip_angle_type(const std::string& type) {
    if (type.size() >= 2 && type.front() == '<' && type.back() == '>') {
        return type.substr(1, type.size() - 2);
    }
    return type;
}

std::string cxx_string_literal(const std::string& text) {
    std::string out = "\"";
    for (char c : text) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    out += '"';
    return out;
}

void add_terminal(Grammar& grammar, const std::string& name, const std::string& type = "") {
    auto it = grammar.terminal_by_name.find(name);
    if (it != grammar.terminal_by_name.end()) {
        if (!type.empty()) {
            grammar.terminals[static_cast<std::size_t>(it->second)].type = type;
        }
        return;
    }

    const int index = static_cast<int>(grammar.terminals.size());
    grammar.terminal_by_name[name] = index;
    grammar.terminals.push_back(SymbolSpec {
        .name = name,
        .terminal = true,
        .type = type,
    });
}

void add_nonterminal(Grammar& grammar, const std::string& name) {
    if (grammar.nonterminal_by_name.find(name) != grammar.nonterminal_by_name.end()) {
        return;
    }

    const int index = static_cast<int>(grammar.nonterminals.size());
    grammar.nonterminal_by_name[name] = index;
    grammar.nonterminals.push_back(SymbolSpec {
        .name = name,
        .terminal = false,
        .type = "",
    });
}

void set_type(Grammar& grammar, const std::string& name, const std::string& type) {
    auto term = grammar.terminal_by_name.find(name);
    if (term != grammar.terminal_by_name.end()) {
        grammar.terminals[static_cast<std::size_t>(term->second)].type = type;
        return;
    }

    add_nonterminal(grammar, name);
    grammar.nonterminals[static_cast<std::size_t>(grammar.nonterminal_by_name.at(name))].type = type;
}

void set_precedence(Grammar& grammar, const std::string& name, int precedence, Assoc assoc) {
    add_terminal(grammar, name);
    auto& symbol = grammar.terminals[static_cast<std::size_t>(grammar.terminal_by_name.at(name))];
    symbol.precedence = precedence;
    symbol.assoc = assoc;
}

void parse_directives(const std::string& directives, Grammar& grammar) {
    std::istringstream input(directives);
    std::string line;
    int precedence_level = 1;

    while (std::getline(input, line)) {
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        const auto words = split_words(line);
        if (words.empty()) {
            continue;
        }

        if (words[0] == "%start") {
            if (words.size() != 2) {
                throw std::runtime_error("%start expects one symbol");
            }
            grammar.start = words[1];
            continue;
        }

        if (words[0] == "%token") {
            std::string type;
            std::size_t index = 1;
            if (index < words.size() && starts_with(words[index], "<")) {
                type = strip_angle_type(words[index]);
                ++index;
            }
            for (; index < words.size(); ++index) {
                if (is_number_token(words[index])) {
                    continue;
                }
                add_terminal(grammar, words[index], type);
            }
            continue;
        }

        if (words[0] == "%type") {
            if (words.size() < 3) {
                throw std::runtime_error("%type expects a type and at least one symbol");
            }
            const std::string type = strip_angle_type(words[1]);
            for (std::size_t i = 2; i < words.size(); ++i) {
                set_type(grammar, words[i], type);
            }
            continue;
        }

        if (words[0] == "%left" || words[0] == "%right" || words[0] == "%nonassoc") {
            Assoc assoc = Assoc::Left;
            if (words[0] == "%right") {
                assoc = Assoc::Right;
            } else if (words[0] == "%nonassoc") {
                assoc = Assoc::Nonassoc;
            }
            for (std::size_t i = 1; i < words.size(); ++i) {
                set_precedence(grammar, words[i], precedence_level, assoc);
            }
            ++precedence_level;
            continue;
        }

        throw std::runtime_error("unknown directive: " + words[0]);
    }

    if (grammar.start.empty()) {
        throw std::runtime_error("missing %start");
    }
}

class ProductionParser {
public:
    ProductionParser(std::string input, Grammar& grammar)
        : input_(std::move(input)), grammar_(grammar) {}

    void parse() {
        while (true) {
            skip_ws_and_comments();
            if (pos_ >= input_.size()) {
                break;
            }

            const std::string lhs = parse_identifier();
            if (lhs.empty()) {
                throw std::runtime_error("expected production lhs");
            }
            add_nonterminal(grammar_, lhs);

            skip_ws_and_comments();
            expect(':');

            while (true) {
                Production prod;
                prod.id = static_cast<int>(grammar_.productions.size()) + 1;
                prod.lhs_name = lhs;
                parse_alternative(prod);
                grammar_.productions.push_back(std::move(prod));

                skip_ws_and_comments();
                if (peek() == '|') {
                    ++pos_;
                    continue;
                }
                if (peek() == ';') {
                    ++pos_;
                    break;
                }
                throw std::runtime_error("expected | or ; after production alternative");
            }
        }
    }

private:
    char peek() const {
        return pos_ < input_.size() ? input_[pos_] : '\0';
    }

    bool match(const std::string& text) const {
        return input_.compare(pos_, text.size(), text) == 0;
    }

    void expect(char expected) {
        if (peek() != expected) {
            std::string message = "expected '";
            message += expected;
            message += "'";
            throw std::runtime_error(message);
        }
        ++pos_;
    }

    void skip_ws_and_comments() {
        while (pos_ < input_.size()) {
            const char c = input_[pos_];
            if (std::isspace(static_cast<unsigned char>(c))) {
                ++pos_;
                continue;
            }
            if (c == '#') {
                while (pos_ < input_.size() && input_[pos_] != '\n') {
                    ++pos_;
                }
                continue;
            }
            if (match("//")) {
                while (pos_ < input_.size() && input_[pos_] != '\n') {
                    ++pos_;
                }
                continue;
            }
            break;
        }
    }

    std::string parse_identifier() {
        if (pos_ >= input_.size()) {
            return "";
        }
        const char first = input_[pos_];
        if (!(std::isalpha(static_cast<unsigned char>(first)) || first == '_')) {
            return "";
        }
        const std::size_t start = pos_++;
        while (pos_ < input_.size()) {
            const char c = input_[pos_];
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
                break;
            }
            ++pos_;
        }
        return input_.substr(start, pos_ - start);
    }

    std::string parse_action_block() {
        if (peek() != '{') {
            throw std::runtime_error("expected action block");
        }

        const std::size_t start = pos_;
        int depth = 0;
        while (pos_ < input_.size()) {
            const char c = input_[pos_++];
            if (c == '"' || c == '\'') {
                skip_quoted(c);
                continue;
            }
            if (c == '/' && pos_ < input_.size() && input_[pos_] == '/') {
                ++pos_;
                while (pos_ < input_.size() && input_[pos_] != '\n') {
                    ++pos_;
                }
                continue;
            }
            if (c == '/' && pos_ < input_.size() && input_[pos_] == '*') {
                ++pos_;
                while (pos_ + 1 < input_.size() &&
                       !(input_[pos_] == '*' && input_[pos_ + 1] == '/')) {
                    ++pos_;
                }
                if (pos_ + 1 < input_.size()) {
                    pos_ += 2;
                }
                continue;
            }
            if (c == '{') {
                ++depth;
            } else if (c == '}') {
                --depth;
                if (depth == 0) {
                    return input_.substr(start, pos_ - start);
                }
            }
        }

        throw std::runtime_error("unterminated action block");
    }

    void skip_quoted(char quote) {
        while (pos_ < input_.size()) {
            const char c = input_[pos_++];
            if (c == '\\') {
                if (pos_ < input_.size()) {
                    ++pos_;
                }
                continue;
            }
            if (c == quote) {
                return;
            }
        }
    }

    void parse_alternative(Production& prod) {
        while (true) {
            skip_ws_and_comments();
            const char c = peek();
            if (c == '|' || c == ';' || c == '\0') {
                return;
            }
            if (c == '{') {
                prod.action = parse_action_block();
                continue;
            }
            if (match("%prec")) {
                pos_ += 5;
                skip_ws_and_comments();
                prod.explicit_prec = parse_identifier();
                if (prod.explicit_prec.empty()) {
                    throw std::runtime_error("expected token after %prec");
                }
                continue;
            }
            const std::string symbol = parse_identifier();
            if (symbol.empty()) {
                throw std::runtime_error("expected rhs symbol");
            }
            prod.rhs_names.push_back(symbol);
        }
    }

    std::string input_;
    Grammar& grammar_;
    std::size_t pos_ = 0;
};

void parse_grammar_file(const std::string& content, Grammar& grammar) {
    const std::size_t delimiter = content.find("%%");
    if (delimiter == std::string::npos) {
        throw std::runtime_error("grammar file is missing %% delimiter");
    }

    parse_directives(content.substr(0, delimiter), grammar);
    ProductionParser parser(content.substr(delimiter + 2), grammar);
    parser.parse();
}

int symbol_id(const Grammar& grammar, const std::string& name) {
    auto terminal = grammar.terminal_by_name.find(name);
    if (terminal != grammar.terminal_by_name.end()) {
        return grammar.terminals[static_cast<std::size_t>(terminal->second)].id;
    }

    auto nonterminal = grammar.nonterminal_by_name.find(name);
    if (nonterminal != grammar.nonterminal_by_name.end()) {
        return grammar.nonterminals[static_cast<std::size_t>(nonterminal->second)].id;
    }

    throw std::runtime_error("unknown symbol: " + name);
}

bool is_terminal(const Grammar& grammar, int sym) {
    return sym >= 0 && sym < grammar.terminal_count();
}

bool is_nonterminal(const Grammar& grammar, int sym) {
    return sym >= grammar.terminal_count() &&
           sym < grammar.terminal_count() + grammar.nonterminal_count();
}

std::string symbol_name(const Grammar& grammar, int sym) {
    if (sym >= 0 && sym < grammar.terminal_count()) {
        return grammar.terminals[static_cast<std::size_t>(sym)].name;
    }
    const int nt = sym - grammar.terminal_count();
    if (nt >= 0 && nt < grammar.nonterminal_count()) {
        return grammar.nonterminals[static_cast<std::size_t>(nt)].name;
    }
    if (sym == grammar.accept_symbol) {
        return "$accept";
    }
    return "<invalid>";
}

const SymbolSpec& terminal_spec(const Grammar& grammar, int sym) {
    return grammar.terminals[static_cast<std::size_t>(sym)];
}

void resolve_symbols(Grammar& grammar) {
    if (grammar.terminal_by_name.find("END") == grammar.terminal_by_name.end()) {
        throw std::runtime_error("missing END terminal");
    }

    for (auto& prod : grammar.productions) {
        add_nonterminal(grammar, prod.lhs_name);
    }

    for (std::size_t i = 0; i < grammar.terminals.size(); ++i) {
        grammar.terminals[i].id = static_cast<int>(i);
        grammar.terminal_by_name[grammar.terminals[i].name] = static_cast<int>(i);
    }
    for (std::size_t i = 0; i < grammar.nonterminals.size(); ++i) {
        grammar.nonterminals[i].id = grammar.terminal_count() + static_cast<int>(i);
        grammar.nonterminal_by_name[grammar.nonterminals[i].name] = static_cast<int>(i);
    }

    grammar.end_symbol = symbol_id(grammar, "END");
    if (grammar.end_symbol != 0) {
        throw std::runtime_error("END must be terminal id 0");
    }

    grammar.accept_symbol = grammar.terminal_count() + grammar.nonterminal_count();

    for (const auto& symbol : grammar.terminals) {
        if (!symbol.type.empty()) {
            grammar.type_by_symbol[symbol.id] = symbol.type;
        }
    }
    for (const auto& symbol : grammar.nonterminals) {
        if (!symbol.type.empty()) {
            grammar.type_by_symbol[symbol.id] = symbol.type;
        }
    }

    for (auto& prod : grammar.productions) {
        prod.lhs = symbol_id(grammar, prod.lhs_name);
        prod.rhs.clear();
        for (const auto& name : prod.rhs_names) {
            prod.rhs.push_back(symbol_id(grammar, name));
        }

        std::string prec_symbol = prod.explicit_prec;
        if (prec_symbol.empty()) {
            for (auto it = prod.rhs.rbegin(); it != prod.rhs.rend(); ++it) {
                if (is_terminal(grammar, *it) &&
                    terminal_spec(grammar, *it).precedence > 0) {
                    prec_symbol = symbol_name(grammar, *it);
                    break;
                }
            }
        }

        if (!prec_symbol.empty()) {
            const int prec_id = symbol_id(grammar, prec_symbol);
            if (!is_terminal(grammar, prec_id)) {
                throw std::runtime_error("%prec symbol is not a terminal: " + prec_symbol);
            }
            prod.precedence = terminal_spec(grammar, prec_id).precedence;
            prod.assoc = terminal_spec(grammar, prec_id).assoc;
        }
    }
}

std::vector<Production> all_productions(const Grammar& grammar) {
    std::vector<Production> result;
    Production accept;
    accept.id = 0;
    accept.lhs_name = "$accept";
    accept.lhs = grammar.accept_symbol;
    accept.rhs = { symbol_id(grammar, grammar.start), grammar.end_symbol };
    accept.rhs_names = { grammar.start, "END" };
    result.push_back(std::move(accept));
    for (const auto& prod : grammar.productions) {
        result.push_back(prod);
    }
    return result;
}

struct FirstSets {
    std::vector<bool> nullable;
    std::vector<std::set<int>> first;
};

FirstSets compute_first_sets(const Grammar& grammar,
                             const std::vector<Production>& productions) {
    FirstSets sets;
    sets.nullable.assign(static_cast<std::size_t>(grammar.symbol_count()), false);
    sets.first.resize(static_cast<std::size_t>(grammar.symbol_count()));

    for (int term = 0; term < grammar.terminal_count(); ++term) {
        sets.first[static_cast<std::size_t>(term)].insert(term);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            if (prod.lhs == grammar.accept_symbol) {
                continue;
            }

            bool rhs_nullable = true;
            for (int sym : prod.rhs) {
                const auto before = sets.first[static_cast<std::size_t>(prod.lhs)].size();
                sets.first[static_cast<std::size_t>(prod.lhs)].insert(
                    sets.first[static_cast<std::size_t>(sym)].begin(),
                    sets.first[static_cast<std::size_t>(sym)].end());
                if (sets.first[static_cast<std::size_t>(prod.lhs)].size() != before) {
                    changed = true;
                }
                if (!sets.nullable[static_cast<std::size_t>(sym)]) {
                    rhs_nullable = false;
                    break;
                }
            }
            if (rhs_nullable && !sets.nullable[static_cast<std::size_t>(prod.lhs)]) {
                sets.nullable[static_cast<std::size_t>(prod.lhs)] = true;
                changed = true;
            }
        }
    }

    return sets;
}

std::set<int> first_sequence(const FirstSets& sets,
                             const std::vector<int>& sequence,
                             int lookahead) {
    std::set<int> result;
    bool nullable_prefix = true;
    for (int sym : sequence) {
        result.insert(sets.first[static_cast<std::size_t>(sym)].begin(),
                      sets.first[static_cast<std::size_t>(sym)].end());
        if (!sets.nullable[static_cast<std::size_t>(sym)]) {
            nullable_prefix = false;
            break;
        }
    }
    if (nullable_prefix) {
        result.insert(lookahead);
    }
    return result;
}

std::set<Item> closure(const Grammar& grammar,
                       const std::vector<Production>& productions,
                       const std::vector<std::vector<int>>& productions_by_lhs,
                       const FirstSets& first_sets,
                       std::set<Item> items) {
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<Item> snapshot(items.begin(), items.end());
        for (const auto& item : snapshot) {
            const auto& prod = productions[static_cast<std::size_t>(item.production)];
            if (item.dot >= static_cast<int>(prod.rhs.size())) {
                continue;
            }

            const int next = prod.rhs[static_cast<std::size_t>(item.dot)];
            if (!is_nonterminal(grammar, next)) {
                continue;
            }

            std::vector<int> suffix;
            for (std::size_t i = static_cast<std::size_t>(item.dot + 1);
                 i < prod.rhs.size(); ++i) {
                suffix.push_back(prod.rhs[i]);
            }

            const auto lookaheads = first_sequence(first_sets, suffix, item.lookahead);
            for (int production_id : productions_by_lhs[static_cast<std::size_t>(next)]) {
                for (int la : lookaheads) {
                    const Item next_item { production_id, 0, la };
                    if (items.insert(next_item).second) {
                        changed = true;
                    }
                }
            }
        }
    }
    return items;
}

std::set<Item> go_to_items(const Grammar& grammar,
                           const std::vector<Production>& productions,
                           const std::vector<std::vector<int>>& productions_by_lhs,
                           const FirstSets& first_sets,
                           const std::set<Item>& items,
                           int symbol) {
    std::set<Item> shifted;
    for (const auto& item : items) {
        const auto& prod = productions[static_cast<std::size_t>(item.production)];
        if (item.dot < static_cast<int>(prod.rhs.size()) &&
            prod.rhs[static_cast<std::size_t>(item.dot)] == symbol) {
            shifted.insert(Item { item.production, item.dot + 1, item.lookahead });
        }
    }
    if (shifted.empty()) {
        return shifted;
    }
    return closure(grammar, productions, productions_by_lhs, first_sets, std::move(shifted));
}

struct Automaton {
    std::vector<std::set<Item>> states;
    std::vector<std::map<int, int>> transitions;
};

Automaton build_lr1_automaton(const Grammar& grammar,
                              const std::vector<Production>& productions,
                              const std::vector<std::vector<int>>& productions_by_lhs,
                              const FirstSets& first_sets) {
    Automaton automaton;
    std::map<std::set<Item>, int> state_by_items;
    std::queue<int> pending;

    std::set<Item> start_items = closure(
        grammar,
        productions,
        productions_by_lhs,
        first_sets,
        std::set<Item> { Item { 0, 0, grammar.end_symbol } });

    automaton.states.push_back(start_items);
    automaton.transitions.emplace_back();
    state_by_items[start_items] = 0;
    pending.push(0);

    while (!pending.empty()) {
        const int state = pending.front();
        pending.pop();

        std::set<int> next_symbols;
        for (const auto& item : automaton.states[static_cast<std::size_t>(state)]) {
            const auto& prod = productions[static_cast<std::size_t>(item.production)];
            if (item.dot < static_cast<int>(prod.rhs.size())) {
                next_symbols.insert(prod.rhs[static_cast<std::size_t>(item.dot)]);
            }
        }

        for (int sym : next_symbols) {
            const auto target_items = go_to_items(
                grammar,
                productions,
                productions_by_lhs,
                first_sets,
                automaton.states[static_cast<std::size_t>(state)],
                sym);
            if (target_items.empty()) {
                continue;
            }

            auto found = state_by_items.find(target_items);
            int target = -1;
            if (found == state_by_items.end()) {
                target = static_cast<int>(automaton.states.size());
                automaton.states.push_back(target_items);
                automaton.transitions.emplace_back();
                state_by_items[target_items] = target;
                pending.push(target);
            } else {
                target = found->second;
            }
            automaton.transitions[static_cast<std::size_t>(state)][sym] = target;
        }
    }

    return automaton;
}

std::set<CoreItem> core_of(const std::set<Item>& items) {
    std::set<CoreItem> core;
    for (const auto& item : items) {
        core.insert(CoreItem { item.production, item.dot });
    }
    return core;
}

Automaton merge_lalr_states(const Automaton& lr1) {
    std::map<std::set<CoreItem>, int> state_by_core;
    std::vector<int> canonical_to_lalr(lr1.states.size(), -1);
    Automaton lalr;
    std::vector<std::map<CoreItem, std::set<int>>> merged_items;

    for (std::size_t state = 0; state < lr1.states.size(); ++state) {
        const auto core = core_of(lr1.states[state]);
        auto found = state_by_core.find(core);
        int target = -1;
        if (found == state_by_core.end()) {
            target = static_cast<int>(lalr.states.size());
            state_by_core[core] = target;
            lalr.states.emplace_back();
            lalr.transitions.emplace_back();
            merged_items.emplace_back();
        } else {
            target = found->second;
        }
        canonical_to_lalr[state] = target;

        for (const auto& item : lr1.states[state]) {
            merged_items[static_cast<std::size_t>(target)]
                [CoreItem { item.production, item.dot }]
                    .insert(item.lookahead);
        }
    }

    for (std::size_t state = 0; state < merged_items.size(); ++state) {
        for (const auto& [core, lookaheads] : merged_items[state]) {
            for (int lookahead : lookaheads) {
                lalr.states[state].insert(Item { core.production, core.dot, lookahead });
            }
        }
    }

    for (std::size_t state = 0; state < lr1.transitions.size(); ++state) {
        const int from = canonical_to_lalr[state];
        for (const auto& [sym, target] : lr1.transitions[state]) {
            const int to = canonical_to_lalr[static_cast<std::size_t>(target)];
            auto& slot = lalr.transitions[static_cast<std::size_t>(from)][sym];
            if (slot != 0 || from == 0) {
                slot = to;
            } else {
                slot = to;
            }
        }
    }

    return lalr;
}

int shift_rule_for(const std::vector<Production>& productions,
                   const std::set<Item>& state_items,
                   int terminal) {
    int best = INT_MAX;
    for (const auto& item : state_items) {
        const auto& prod = productions[static_cast<std::size_t>(item.production)];
        if (item.dot < static_cast<int>(prod.rhs.size()) &&
            prod.rhs[static_cast<std::size_t>(item.dot)] == terminal) {
            best = std::min(best, item.production);
        }
    }
    return best == INT_MAX ? -1 : best;
}

void record_sr_conflict(std::vector<Conflict>& conflicts,
                        const Grammar& grammar,
                        int state,
                        int token,
                        int reduce_rule,
                        int shift_rule) {
    conflicts.push_back(Conflict {
        .kind = "sr",
        .token = symbol_name(grammar, token),
        .state = state,
        .reduce_rule = reduce_rule,
        .shift_rule = shift_rule,
    });
}

void record_rr_conflict(std::vector<Conflict>& conflicts,
                        const Grammar& grammar,
                        int state,
                        int token,
                        int keep_rule,
                        int drop_rule) {
    conflicts.push_back(Conflict {
        .kind = "rr",
        .token = symbol_name(grammar, token),
        .state = state,
        .reduce_rule = keep_rule,
        .other_rule = drop_rule,
    });
}

TableAction resolve_shift_reduce(const Grammar& grammar,
                                 const std::vector<Production>& productions,
                                 std::vector<Conflict>& conflicts,
                                 int state,
                                 int token,
                                 const TableAction& shift,
                                 const TableAction& reduce) {
    const int token_prec = terminal_spec(grammar, token).precedence;
    const int rule_prec = productions[static_cast<std::size_t>(reduce.rule)].precedence;

    if (token_prec > 0 && rule_prec > 0) {
        if (token_prec > rule_prec) {
            return shift;
        }
        if (rule_prec > token_prec) {
            return reduce;
        }

        const Assoc assoc = terminal_spec(grammar, token).assoc;
        if (assoc == Assoc::Left) {
            return reduce;
        }
        if (assoc == Assoc::Right) {
            return shift;
        }
        return TableAction {};
    }

    record_sr_conflict(conflicts, grammar, state, token, reduce.rule, shift.shift_rule);
    return shift;
}

void set_action(const Grammar& grammar,
                const std::vector<Production>& productions,
                Tables& tables,
                int state,
                int token,
                const TableAction& incoming) {
    auto& current = tables.action[static_cast<std::size_t>(state)]
                                [static_cast<std::size_t>(token)];

    if (current.kind == ActionKind::Error) {
        current = incoming;
        return;
    }

    if (current.kind == incoming.kind &&
        current.target == incoming.target &&
        current.rule == incoming.rule) {
        return;
    }

    if (current.kind == ActionKind::Shift && incoming.kind == ActionKind::Reduce) {
        current = resolve_shift_reduce(
            grammar, productions, tables.conflicts, state, token, current, incoming);
        return;
    }

    if (current.kind == ActionKind::Reduce && incoming.kind == ActionKind::Shift) {
        current = resolve_shift_reduce(
            grammar, productions, tables.conflicts, state, token, incoming, current);
        return;
    }

    if (current.kind == ActionKind::Reduce && incoming.kind == ActionKind::Reduce) {
        const int keep = std::min(current.rule, incoming.rule);
        const int drop = std::max(current.rule, incoming.rule);
        record_rr_conflict(tables.conflicts, grammar, state, token, keep, drop);
        current = TableAction { .kind = ActionKind::Reduce, .rule = keep };
        return;
    }

    if (incoming.kind == ActionKind::Accept) {
        current = incoming;
    }
}

Tables build_tables(const Grammar& grammar,
                    const std::vector<Production>& productions,
                    const Automaton& automaton) {
    Tables tables;
    tables.action.assign(
        automaton.states.size(),
        std::vector<TableAction>(static_cast<std::size_t>(grammar.terminal_count())));
    tables.go_to.assign(
        automaton.states.size(),
        std::vector<int>(static_cast<std::size_t>(grammar.nonterminal_count()), -1));

    for (std::size_t state = 0; state < automaton.states.size(); ++state) {
        for (const auto& [sym, target] : automaton.transitions[state]) {
            if (is_terminal(grammar, sym)) {
                set_action(
                    grammar,
                    productions,
                    tables,
                    static_cast<int>(state),
                    sym,
                    TableAction {
                        .kind = ActionKind::Shift,
                        .target = target,
                        .shift_rule = shift_rule_for(productions, automaton.states[state], sym),
                    });
            } else if (is_nonterminal(grammar, sym)) {
                tables.go_to[state][static_cast<std::size_t>(sym - grammar.terminal_count())] = target;
            }
        }

        for (const auto& item : automaton.states[state]) {
            const auto& prod = productions[static_cast<std::size_t>(item.production)];
            if (item.dot != static_cast<int>(prod.rhs.size())) {
                continue;
            }
            if (item.production == 0) {
                set_action(
                    grammar,
                    productions,
                    tables,
                    static_cast<int>(state),
                    grammar.end_symbol,
                    TableAction { .kind = ActionKind::Accept });
            } else {
                set_action(
                    grammar,
                    productions,
                    tables,
                    static_cast<int>(state),
                    item.lookahead,
                    TableAction {
                        .kind = ActionKind::Reduce,
                        .rule = item.production,
                    });
            }
        }
    }

    return tables;
}

std::string strip_outer_braces(const std::string& action) {
    std::string body = trim(action);
    if (body.size() >= 2 && body.front() == '{' && body.back() == '}') {
        body = body.substr(1, body.size() - 2);
    }
    return body;
}

std::string rhs_type(const Grammar& grammar, const Production& prod, int index) {
    if (index < 1 || index > static_cast<int>(prod.rhs.size())) {
        throw std::runtime_error("semantic value index out of range");
    }
    const int sym = prod.rhs[static_cast<std::size_t>(index - 1)];
    auto found = grammar.type_by_symbol.find(sym);
    if (found == grammar.type_by_symbol.end()) {
        throw std::runtime_error("symbol has no semantic type: " + symbol_name(grammar, sym));
    }
    return found->second;
}

void copy_quoted(const std::string& input, std::size_t& i, std::ostringstream& out) {
    const char quote = input[i];
    out << quote;
    ++i;
    while (i < input.size()) {
        const char c = input[i++];
        out << c;
        if (c == '\\' && i < input.size()) {
            out << input[i++];
            continue;
        }
        if (c == quote) {
            return;
        }
    }
}

std::string transform_action(const Grammar& grammar,
                             const Production& prod,
                             const std::string& body) {
    std::ostringstream out;
    for (std::size_t i = 0; i < body.size();) {
        const char c = body[i];

        if (c == '"' || c == '\'') {
            copy_quoted(body, i, out);
            continue;
        }

        if (c == '/' && i + 1 < body.size() && body[i + 1] == '/') {
            while (i < body.size() && body[i] != '\n') {
                out << body[i++];
            }
            continue;
        }

        if (c == '/' && i + 1 < body.size() && body[i + 1] == '*') {
            out << body[i];
            ++i;
            out << body[i];
            ++i;
            while (i + 1 < body.size() && !(body[i] == '*' && body[i + 1] == '/')) {
                out << body[i++];
            }
            if (i + 1 < body.size()) {
                out << body[i];
                ++i;
                out << body[i];
                ++i;
            }
            continue;
        }

        if (c == '$') {
            if (i + 1 < body.size() && body[i + 1] == '$') {
                out << "yyresult";
                i += 2;
                continue;
            }

            std::size_t j = i + 1;
            while (j < body.size() && std::isdigit(static_cast<unsigned char>(body[j]))) {
                ++j;
            }
            if (j == i + 1) {
                out << c;
                ++i;
                continue;
            }
            const int index = std::stoi(body.substr(i + 1, j - (i + 1)));
            out << "value_at<" << rhs_type(grammar, prod, index)
                << ">(stack, base, " << index << ")";
            i = j;
            continue;
        }

        if (c == '@') {
            if (i + 1 < body.size() && body[i + 1] == '$') {
                out << "result_span";
                i += 2;
                continue;
            }

            std::size_t j = i + 1;
            while (j < body.size() && std::isdigit(static_cast<unsigned char>(body[j]))) {
                ++j;
            }
            if (j == i + 1) {
                out << c;
                ++i;
                continue;
            }
            const int index = std::stoi(body.substr(i + 1, j - (i + 1)));
            out << "span_at(stack, base, " << index << ")";
            i = j;
            continue;
        }

        out << c;
        ++i;
    }
    return out.str();
}

std::vector<std::string> expected_conflicts(const std::string& path) {
    std::vector<std::string> expected;
    if (path.empty()) {
        return expected;
    }

    std::istringstream input(read_file(path));
    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const auto words = split_words(line);
        if (words.empty()) {
            continue;
        }
        if (words[0] == "sr") {
            std::string token;
            std::string reduce;
            std::string shift;
            for (const auto& word : words) {
                if (starts_with(word, "token=")) token = word.substr(6);
                if (starts_with(word, "reduce_rule=")) reduce = word.substr(12);
                if (starts_with(word, "shift_rule=")) shift = word.substr(11);
            }
            expected.push_back("sr token=" + token +
                               " reduce_rule=" + reduce +
                               " shift_rule=" + shift);
        } else if (words[0] == "rr") {
            std::string token;
            std::string keep;
            std::string drop;
            for (const auto& word : words) {
                if (starts_with(word, "token=")) token = word.substr(6);
                if (starts_with(word, "keep_rule=")) keep = word.substr(10);
                if (starts_with(word, "drop_rule=")) drop = word.substr(10);
            }
            expected.push_back("rr token=" + token +
                               " keep_rule=" + keep +
                               " drop_rule=" + drop);
        }
    }

    std::sort(expected.begin(), expected.end());
    return expected;
}

void verify_conflicts(const std::vector<Conflict>& conflicts,
                      const std::string& expected_path) {
    std::vector<std::string> actual;
    actual.reserve(conflicts.size());
    for (const auto& conflict : conflicts) {
        actual.push_back(conflict.normalized());
    }
    std::sort(actual.begin(), actual.end());

    const auto expected = expected_conflicts(expected_path);
    if (!expected_path.empty() && actual != expected) {
        std::cerr << "parsergen: conflict baseline mismatch\n";
        std::cerr << "expected:\n";
        for (const auto& line : expected) {
            std::cerr << "  " << line << '\n';
        }
        std::cerr << "actual:\n";
        for (const auto& line : actual) {
            std::cerr << "  " << line << '\n';
        }
        throw std::runtime_error("conflict baseline mismatch");
    }
}

void print_conflicts(const std::vector<Conflict>& conflicts) {
    std::cerr << "parsergen: " << conflicts.size() << " conflict(s)\n";
    for (const auto& conflict : conflicts) {
        std::cerr << "  " << conflict.normalized()
                  << " state=" << conflict.state << '\n';
    }
}

int encode_action(const TableAction& action) {
    if (action.kind == ActionKind::Error) return 0;
    if (action.kind == ActionKind::Accept) return INT_MAX;
    if (action.kind == ActionKind::Shift) return action.target + 1;
    if (action.kind == ActionKind::Reduce) return -action.rule;
    return 0;
}

std::string emit_header(const Grammar& grammar,
                        const std::vector<Production>& productions,
                        const Automaton& automaton) {
    std::ostringstream out;
    out << "#pragma once\n\n";
    out << "#include <vector>\n\n";
    out << "#include \"../common/span.hpp\"\n";
    out << "#include \"parser_driver.hpp\"\n";
    out << "#include \"symbol.hpp\"\n\n";
    out << "namespace hulk::parser::parser_tables {\n\n";
    out << "struct symbol_id {\n";
    for (const auto& symbol : grammar.terminals) {
        out << "    static constexpr int " << symbol.name << " = " << symbol.id << ";\n";
    }
    for (const auto& symbol : grammar.nonterminals) {
        out << "    static constexpr int " << symbol.name << " = " << symbol.id << ";\n";
    }
    out << "};\n\n";
    out << "inline constexpr int terminal_count = " << grammar.terminal_count() << ";\n";
    out << "inline constexpr int nonterminal_count = " << grammar.nonterminal_count() << ";\n";
    out << "inline constexpr int symbol_count = " << grammar.symbol_count() << ";\n";
    out << "inline constexpr int state_count = " << automaton.states.size() << ";\n";
    out << "inline constexpr int rule_count = " << productions.size() << ";\n";
    out << "inline constexpr int error_action = 0;\n";
    out << "inline constexpr int accept_action = 2147483647;\n\n";
    out << "int action(int state, int terminal);\n";
    out << "int go_to(int state, int lhs_symbol);\n";
    out << "int rule_lhs(int rule);\n";
    out << "int rule_len(int rule);\n";
    out << "const char* symbol_name(int symbol);\n";
    out << "hulk::common::Span merged_span(const std::vector<StackEntry>& stack,\n";
    out << "                              std::size_t base,\n";
    out << "                              int rhs_len);\n";
    out << "void run_semantic_action(int rule,\n";
    out << "                         std::vector<StackEntry>& stack,\n";
    out << "                         std::size_t base,\n";
    out << "                         hulk::common::Span& result_span,\n";
    out << "                         ParserValue& result_value,\n";
    out << "                         ParserDriver& driver);\n\n";
    out << "} // namespace hulk::parser::parser_tables\n";
    return out.str();
}

std::string emit_source(const Grammar& grammar,
                        const std::vector<Production>& productions,
                        const Tables& tables) {
    std::ostringstream out;
    out << "#include \"parser_tables.hpp\"\n\n";
    out << "#include <climits>\n";
    out << "#include <cstddef>\n";
    out << "#include <memory>\n";
    out << "#include <stdexcept>\n";
    out << "#include <utility>\n";
    out << "#include <variant>\n\n";
    out << "namespace hulk::parser::parser_tables {\n";
    out << "namespace {\n\n";

    out << "constexpr int action_table[state_count][terminal_count] = {\n";
    for (const auto& row : tables.action) {
        out << "    { ";
        for (std::size_t i = 0; i < row.size(); ++i) {
            if (i > 0) out << ", ";
            out << encode_action(row[i]);
        }
        out << " },\n";
    }
    out << "};\n\n";

    out << "constexpr int goto_table[state_count][nonterminal_count] = {\n";
    for (const auto& row : tables.go_to) {
        out << "    { ";
        for (std::size_t i = 0; i < row.size(); ++i) {
            if (i > 0) out << ", ";
            out << row[i];
        }
        out << " },\n";
    }
    out << "};\n\n";

    out << "constexpr int rule_lhs_table[rule_count] = { ";
    for (std::size_t i = 0; i < productions.size(); ++i) {
        if (i > 0) out << ", ";
        out << productions[i].lhs;
    }
    out << " };\n\n";

    out << "constexpr int rule_len_table[rule_count] = { ";
    for (std::size_t i = 0; i < productions.size(); ++i) {
        if (i > 0) out << ", ";
        out << productions[i].rhs.size();
    }
    out << " };\n\n";

    out << "const char* symbol_names[symbol_count] = {\n";
    for (const auto& symbol : grammar.terminals) {
        out << "    " << cxx_string_literal(symbol.name) << ",\n";
    }
    for (const auto& symbol : grammar.nonterminals) {
        out << "    " << cxx_string_literal(symbol.name) << ",\n";
    }
    out << "    \"$accept\",\n";
    out << "};\n\n";

    out << "template <typename T>\n";
    out << "T& value_at(std::vector<StackEntry>& stack, std::size_t base, std::size_t index) {\n";
    out << "    return std::get<T>(stack.at(base + index - 1).value);\n";
    out << "}\n\n";
    out << "const hulk::common::Span& span_at(const std::vector<StackEntry>& stack,\n";
    out << "                                  std::size_t base,\n";
    out << "                                  std::size_t index) {\n";
    out << "    return stack.at(base + index - 1).span;\n";
    out << "}\n\n";
    out << "hulk::common::Span to_span(const hulk::common::Span& span) {\n";
    out << "    return span;\n";
    out << "}\n\n";
    out << "} // namespace\n\n";

    out << "int action(int state, int terminal) {\n";
    out << "    if (state < 0 || state >= state_count || terminal < 0 || terminal >= terminal_count) {\n";
    out << "        return error_action;\n";
    out << "    }\n";
    out << "    return action_table[state][terminal];\n";
    out << "}\n\n";

    out << "int go_to(int state, int lhs_symbol) {\n";
    out << "    const int nonterminal = lhs_symbol - terminal_count;\n";
    out << "    if (state < 0 || state >= state_count || nonterminal < 0 || nonterminal >= nonterminal_count) {\n";
    out << "        return -1;\n";
    out << "    }\n";
    out << "    return goto_table[state][nonterminal];\n";
    out << "}\n\n";

    out << "int rule_lhs(int rule) {\n";
    out << "    if (rule < 0 || rule >= rule_count) return -1;\n";
    out << "    return rule_lhs_table[rule];\n";
    out << "}\n\n";

    out << "int rule_len(int rule) {\n";
    out << "    if (rule < 0 || rule >= rule_count) return -1;\n";
    out << "    return rule_len_table[rule];\n";
    out << "}\n\n";

    out << "const char* symbol_name(int symbol) {\n";
    out << "    if (symbol < 0 || symbol >= symbol_count) return \"<invalid>\";\n";
    out << "    return symbol_names[symbol];\n";
    out << "}\n\n";

    out << "hulk::common::Span merged_span(const std::vector<StackEntry>& stack,\n";
    out << "                              std::size_t base,\n";
    out << "                              int rhs_len) {\n";
    out << "    if (rhs_len > 0) {\n";
    out << "        return hulk::common::Span {\n";
    out << "            .start = stack.at(base).span.start,\n";
    out << "            .end = stack.at(base + static_cast<std::size_t>(rhs_len) - 1).span.end,\n";
    out << "        };\n";
    out << "    }\n";
    out << "    const auto end = base == 0 ? hulk::common::Position {} : stack.at(base - 1).span.end;\n";
    out << "    return hulk::common::Span { .start = end, .end = end };\n";
    out << "}\n\n";

    out << "void run_semantic_action(int rule,\n";
    out << "                         std::vector<StackEntry>& stack,\n";
    out << "                         std::size_t base,\n";
    out << "                         hulk::common::Span& result_span,\n";
    out << "                         ParserValue& result_value,\n";
    out << "                         ParserDriver& driver) {\n";
    out << "    switch (rule) {\n";

    for (std::size_t i = 1; i < productions.size(); ++i) {
        const auto& prod = productions[i];
        const auto lhs_type = grammar.type_by_symbol.find(prod.lhs);
        const bool has_type = lhs_type != grammar.type_by_symbol.end();
        const std::string action_body = strip_outer_braces(prod.action);

        out << "        case " << prod.id << ": {\n";
        if (has_type) {
            out << "            " << lhs_type->second << " yyresult {};\n";
        }

        if (!action_body.empty()) {
            const std::string transformed = transform_action(grammar, prod, action_body);
            std::istringstream body_stream(transformed);
            std::string line;
            while (std::getline(body_stream, line)) {
                out << "            " << line << '\n';
            }
        } else if (has_type && !prod.rhs.empty()) {
            const int first_sym = prod.rhs.front();
            auto first_type = grammar.type_by_symbol.find(first_sym);
            if (first_type != grammar.type_by_symbol.end()) {
                out << "            yyresult = std::move(value_at<"
                    << first_type->second << ">(stack, base, 1));\n";
            }
        }

        if (has_type) {
            out << "            result_value = std::move(yyresult);\n";
        }
        out << "            break;\n";
        out << "        }\n";
    }

    out << "        default:\n";
    out << "            break;\n";
    out << "    }\n";
    out << "}\n\n";
    out << "} // namespace hulk::parser::parser_tables\n";
    return out.str();
}

std::vector<std::vector<int>> productions_by_lhs(const Grammar& grammar,
                                                 const std::vector<Production>& productions) {
    std::vector<std::vector<int>> result(static_cast<std::size_t>(grammar.symbol_count()));
    for (const auto& prod : productions) {
        if (prod.lhs >= 0 && prod.lhs < grammar.symbol_count()) {
            result[static_cast<std::size_t>(prod.lhs)].push_back(prod.id);
        }
    }
    return result;
}

void usage() {
    std::cerr << "usage: parsergen <grammar> -o <output-prefix> [--conflicts <baseline>]\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 4) {
            usage();
            return 2;
        }

        std::string grammar_path;
        std::string output_prefix;
        std::string conflicts_path;

        grammar_path = argv[1];
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc) {
                output_prefix = argv[++i];
            } else if (arg == "--conflicts" && i + 1 < argc) {
                conflicts_path = argv[++i];
            } else {
                usage();
                return 2;
            }
        }

        if (output_prefix.empty()) {
            usage();
            return 2;
        }

        Grammar grammar;
        parse_grammar_file(read_file(grammar_path), grammar);
        resolve_symbols(grammar);

        const auto productions = all_productions(grammar);
        const auto by_lhs = productions_by_lhs(grammar, productions);
        const auto first_sets = compute_first_sets(grammar, productions);
        const auto lr1 = build_lr1_automaton(grammar, productions, by_lhs, first_sets);
        const auto lalr = merge_lalr_states(lr1);
        const auto tables = build_tables(grammar, productions, lalr);

        print_conflicts(tables.conflicts);
        verify_conflicts(tables.conflicts, conflicts_path);

        write_file(output_prefix + ".hpp", emit_header(grammar, productions, lalr));
        write_file(output_prefix + ".cpp", emit_source(grammar, productions, tables));

        std::cerr << "parsergen: emitted " << output_prefix
                  << ".{hpp,cpp} with " << lalr.states.size()
                  << " states and " << productions.size() - 1
                  << " grammar rules\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "parsergen: " << ex.what() << '\n';
        return 1;
    }
}
