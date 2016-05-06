#ifndef SNUMBER_H_INCLUDED
#define SNUMBER_H_INCLUDED
#include <string>

struct SNumber
{
public:
    SNumber() {}

    static SNumber Create(int value, int exp);

    bool Parse(const std::string& input);

    void Output(std::ostream& to) const;

    const std::string& GetNumsBeforePoint() const;
    const std::string& GetNumsAfterPoint() const;

    SNumber operator/(const SNumber& other) const;
    SNumber operator*(const SNumber& other) const;
    SNumber operator+(const SNumber& other) const;
    SNumber operator-(const SNumber& other) const;

    SNumber& operator+=(const SNumber& other);
    SNumber& operator-=(const SNumber& other);
    SNumber& operator*=(const SNumber& other);
    SNumber& operator/=(const SNumber& other);

    bool IsZero() const;

    bool operator==(const SNumber& other) const;
    bool operator!=(const SNumber& other) const;
    bool operator<(const SNumber& other) const;
    bool operator>(const SNumber& other) const;
    bool operator<=(const SNumber& other) const;
    bool operator>=(const SNumber& other) const;

private:
    std::string      numBefPoint = "";
    std::string      numAftPoint = "";
    bool             negative = false;

    static bool IsNumber(char c);

    SNumber MultUChar(unsigned char m) const;

    void Trim();

    void ApplyExponent10(int exp);

    SNumber Sub(const SNumber& other) const;

    static void Borrow10(std::string& s, int pos);

    bool EqualAbs(const SNumber& other) const;

    bool LessAbs(const SNumber& other) const;
};

#endif // SNUMBER_H_INCLUDED
