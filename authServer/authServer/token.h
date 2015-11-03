#pragma once


class tokenGenerator : private boost::noncopyable
{
private:
    uint64_t uid_;
    int durationType_;
    string token_;
    
public:
    tokenGenerator(const uint64_t uid, const int durationType);
    inline const string& token() { return token_; }
    int generate();
};

class tokenParser : private boost::noncopyable
{
private:
    string token_;
    uint64_t uid_;
    bool valid_;
    string begin_;
    string end_;
    
public:
    tokenParser(const string& token);
    int parse();
    inline bool isValid() { return valid_; }
    inline const string& begin() { return begin_; }
    inline const string& end() { return end_; }
    inline const uint64_t uid() { return uid_; }
};