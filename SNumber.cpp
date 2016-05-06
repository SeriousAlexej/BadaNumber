#include "SNumber.h"
#include <iostream>
#include <algorithm>

using namespace std;

SNumber SNumber::Create(int value, int exp)
{
    SNumber ret;
    if(value < 0)
    {
        ret.negative = true;
        value *= -1;
    }
    ret.numBefPoint = to_string(value);
    ret.ApplyExponent10(exp);
    ret.Trim();
    return ret;
}

bool SNumber::Parse(const string& input)
{
    numAftPoint = "";
    numBefPoint = "";
    negative = false;

    enum
    {
        R_SIGN,
        R_BEF_POINT,
        R_AFT_POINT,
        R_EXPONENT,
        R_DONE
    } readPhase = R_SIGN;

    for(unsigned i=0, lim=input.length(); i<lim; ++i)
    {
        switch(readPhase)
        {
        case R_SIGN:
            if(SNumber::IsNumber(input[i]))
            {
                i--; //read input[i] again on next iteration
                negative = false;
            }
            else if(input[i] == '-')
            {
                negative = true;
            }
            else if(input[i] == '+')
            {
                negative = false;
            }
            else
            {
                cout << "Incorrect input!\n";
                return false;
            }
            readPhase = R_BEF_POINT;
            break;

        case R_BEF_POINT:
            if(SNumber::IsNumber(input[i]) && i+1 < lim && (input[i+1] == ',' || input[i+1] == '.'))
            {
                numBefPoint += input[i++]; //skip ',' or '.'
                readPhase = R_AFT_POINT;
                break;
            }
            cout << "Incorrect input!\n";
            return false;

        case R_AFT_POINT:
            if(SNumber::IsNumber(input[i]))
            {
                numAftPoint += input[i];
                break;
            }
            else if(input[i] == 'e' || input[i] == 'E')
            {
                readPhase = R_EXPONENT;
                break;
            }
            cout << "Incorrect input!\n";
            return false;

        case R_EXPONENT:
            {
                int numExp;
                string::size_type sz;
                string leftover = input.substr(i);
                numExp = stoi(leftover,&sz);
                if(sz != leftover.size())
                {
                    cout << "Incorrect input!\n";
                    return 1;
                }
                ApplyExponent10(numExp);
                i = lim;
                readPhase = R_DONE;
                break;
            }
        default:
            cout << "wat\n";
            return false;
        }
    }
    if(readPhase != R_DONE)
    {
        cout << "Incorrect input!\n";
        return false;
    }
    Trim();
    if(IsZero())
    {
        negative = false;
    }
    return true;
}

void SNumber::Output(std::ostream& to) const
{
    if(negative)
        to << '-';

    if(numBefPoint.empty())
        to << '0';
    else
        to << numBefPoint;

    to << '.';

    if(numAftPoint.empty())
        to << '0';
    else
        to << numAftPoint;

    to << endl;
}

string SNumber::ToString() const
{
    string num = "";
    if(negative)
        num += '-';

    if(numBefPoint.empty())
        num += '0';
    else
        num += numBefPoint;

    if(!numAftPoint.empty())
        num += "." + numAftPoint;
    return num;
}

const string& SNumber::GetNumsBeforePoint() const
{
    return numBefPoint;
}

const string& SNumber::GetNumsAfterPoint() const
{
    return numAftPoint;
}

SNumber SNumber::operator/(const SNumber& other) const
{
    if(other.IsZero() || IsZero())
    {
        if(other.IsZero())
            cout << "R U MAD?\n";
        return SNumber();
    }
    SNumber divisor = other;
    SNumber divident = *this;
    SNumber quotient;
    if(negative == divisor.negative)
        quotient.negative = false;
    else
        quotient.negative = true;

    divident.negative = divisor.negative = false;

    int a = max(0, (int)divident.numBefPoint.size() - (int)divisor.numBefPoint.size() - 1);
    SNumber numWhole = SNumber::Create(1, a);
    SNumber tmp = divisor;
    tmp.ApplyExponent10(a);
    while(a >= 0)
    {
        SNumber tmp2 = divisor;
        tmp2.ApplyExponent10(a);
        tmp2 += tmp;
        if(tmp2 > divident)
        {
            a--;
            continue;
        }
        tmp = tmp2;
        numWhole += SNumber::Create(1, a);
    }
    if(tmp > divident)
    {
        tmp -= divisor;
        numWhole -= SNumber::Create(1, 0);
    }

    quotient.numBefPoint = numWhole.numBefPoint;

    divident = divident - tmp;
    divident.ApplyExponent10(1);
    divident.Trim();

    const int maxDigits = 100;
    for(int i=0; i<maxDigits; ++i)
    {
        a = max(0, (int)divident.numBefPoint.size() - (int)divisor.numBefPoint.size() - 1);
        numWhole = SNumber::Create(1, a);
        tmp = divisor;
        tmp.ApplyExponent10(a);
        while(a >= 0)
        {
            SNumber tmp2 = divisor;
            tmp2.ApplyExponent10(a);
            tmp2 += tmp;
            if(tmp2 > divident)
            {
                a--;
                continue;
            }
            tmp = tmp2;
            numWhole += SNumber::Create(1, a);
        }
        if(tmp > divident)
        {
            tmp -= divisor;
            numWhole -= SNumber::Create(1, 0);
        }

        quotient.numAftPoint += (numWhole.numBefPoint.empty() ? "0" : numWhole.numBefPoint);
        divident = divident - tmp;
        if(divident.IsZero())
        {
            break;
        }
        divident.ApplyExponent10(1);
        divident.Trim();
    }

    quotient.Trim();
    return quotient;
}

SNumber SNumber::operator*(const SNumber& other) const
{
    if(IsZero() || other.IsZero())
    {
        return SNumber();
    }
    int resExp = numAftPoint.size() + other.numAftPoint.size();
    SNumber n1, n2, result;
    n1.numBefPoint = numBefPoint + numAftPoint;
    n1.Trim();
    n2.numBefPoint = other.numBefPoint + other.numAftPoint;
    n2.Trim();

    for(int i=0, lim=n2.numBefPoint.size(); i<lim; ++i)
    {
        SNumber tmp = n1.MultUChar(n2.numBefPoint[lim-1-i] - '0');
        tmp.ApplyExponent10(i);
        result = result + tmp;
    }
    result.ApplyExponent10(-resExp);

    if(negative != other.negative)
    {
        result.negative = true;
    }
    result.Trim();
    return result;
}

SNumber SNumber::operator+(const SNumber& other) const
{
    if(IsZero())
        return other;
    if(other.IsZero())
        return *this;
    if(other.negative != negative)
    {
        if(EqualAbs(other))
        {
            return SNumber();
        }
        if(negative && LessAbs(other))
        {
            return other.Sub(*this);
        }
        if(other.negative && other.LessAbs(*this))
        {
            return Sub(other);
        }
        if(negative && !LessAbs(other))
        {
            SNumber res = Sub(other);
            res.negative = true;
            return res;
        }
        if(other.negative && !other.LessAbs(*this))
        {
            SNumber res = other.Sub(*this);
            res.negative = true;
            return res;
        }
    }
    SNumber upper = *this;
    SNumber oth = other;
    int maxBPlen = max(oth.numBefPoint.size(), numBefPoint.size()) + 1;
    int maxAPlen = max(oth.numAftPoint.size(), numAftPoint.size());
    upper.numBefPoint = string(maxBPlen - numBefPoint.size(), '0') + upper.numBefPoint;
    oth.numBefPoint = string(maxBPlen - other.numBefPoint.size(), '0') + oth.numBefPoint;
    upper.numAftPoint += string(maxAPlen - numAftPoint.size(), '0');
    oth.numAftPoint += string(maxAPlen - other.numAftPoint.size(), '0');

    int exponent = -maxAPlen;

    upper.numBefPoint += upper.numAftPoint;
    oth.numBefPoint += oth.numAftPoint;

    string& n1 = upper.numBefPoint;
    string& n2 = oth.numBefPoint;
    string result(n1.size(), '0');

    for(int i=n1.size()-1; i>=0; --i)
    {
        char tmp = n1[i] + (n2[i] - '0');
        if(tmp > '9')
        {
            result[i] = tmp - 10;
            n1[i-1]++;
        }
        else
        {
            result[i] = tmp;
        }
    }

    SNumber ret;
    ret.numBefPoint = result;
    ret.ApplyExponent10(exponent);
    ret.Trim();
    ret.negative = negative;
    return ret;
}

SNumber SNumber::operator-(const SNumber& other) const
{
    SNumber minOther = other;
    minOther.negative = !minOther.negative;
    return (*this) + minOther;
}

SNumber& SNumber::operator+=(const SNumber& other)
{
    (*this) = (*this) + other;
    return *this;
}

SNumber& SNumber::operator-=(const SNumber& other)
{
    (*this) = (*this) - other;
    return *this;
}

SNumber& SNumber::operator*=(const SNumber& other)
{
    (*this) = (*this) * other;
    return *this;
}

SNumber& SNumber::operator/=(const SNumber& other)
{
    (*this) = (*this) / other;
    return *this;
}

bool SNumber::IsZero() const
{
    return numBefPoint.empty() && numAftPoint.empty();
}

bool SNumber::operator==(const SNumber& other) const
{
    return (negative == other.negative && numBefPoint == other.numBefPoint && numAftPoint == other.numAftPoint)
        || (IsZero() && other.IsZero());
}

bool SNumber::operator!=(const SNumber& other) const
{
    return !(*this == other);
}

bool SNumber::operator<(const SNumber& other) const
{
    if(*this == other)
    {
        return false;
    }
    if(IsZero())
        return !other.negative;
    if(other.IsZero())
        return negative;
    if(negative != other.negative)
    {
        return negative;
    }
    if(!negative)
        return LessAbs(other);
    return !LessAbs(other);
}

bool SNumber::operator>(const SNumber& other) const
{
    return (*this != other) && !(*this < other);
}

bool SNumber::operator<=(const SNumber& other) const
{
    return (*this == other) || (*this < other);
}

bool SNumber::operator>=(const SNumber& other) const
{
    return !(*this < other);
}

bool SNumber::IsNumber(char c)
{
    return c >= '0' && c <= '9';
}

SNumber SNumber::MultUChar(unsigned char m) const
{
    SNumber res;
    for(unsigned char i=0; i<m; ++i)
        res = res + (*this);
    return res;
}

void SNumber::Trim()
{
    int i, l;
    for(i=0, l=numBefPoint.size(); i<l && numBefPoint[i] == '0'; ++i);
    numBefPoint = numBefPoint.substr(i);
    for(i=0, l=numAftPoint.size(); i<l && numAftPoint[l-1-i] == '0'; ++i);
    numAftPoint = numAftPoint.substr(0, l-i);
}

void SNumber::ApplyExponent10(int exp)
{
    if(exp > 0)
    {
        numBefPoint += numAftPoint.substr(0, min(exp, (int)numAftPoint.size() ));

        if((unsigned)exp > numAftPoint.size())
            numBefPoint += string(exp - numAftPoint.size(), '0');

        numAftPoint = numAftPoint.substr(min(exp, (int)numAftPoint.size() ));
    }
    else if(exp < 0)
    {
        exp *= -1;
        numAftPoint = numBefPoint.substr(max(0, (int)numBefPoint.size() - exp )) + numAftPoint;

        if((unsigned)exp > numBefPoint.size())
            numAftPoint = string(exp - numBefPoint.size(), '0') + numAftPoint;

        numBefPoint = numBefPoint.substr(0, max(0, (int)numBefPoint.size() - exp ));
    }
}

SNumber SNumber::Sub(const SNumber& other) const
{
    SNumber upper = *this;
    SNumber oth = other;
    int maxBPlen = max(oth.numBefPoint.size(), numBefPoint.size());
    int maxAPlen = max(oth.numAftPoint.size(), numAftPoint.size());
    upper.numBefPoint = string(maxBPlen - numBefPoint.size(), '0') + upper.numBefPoint;
    oth.numBefPoint = string(maxBPlen - other.numBefPoint.size(), '0') + oth.numBefPoint;
    upper.numAftPoint += string(maxAPlen - numAftPoint.size(), '0');
    oth.numAftPoint += string(maxAPlen - other.numAftPoint.size(), '0');

    int exponent = -maxAPlen;

    upper.numBefPoint += upper.numAftPoint;
    oth.numBefPoint += oth.numAftPoint;

    string& n1 = upper.numBefPoint;
    string& n2 = oth.numBefPoint;
    string result(n1.size(), '0');

    for(int i=n1.size()-1; i>=0; --i)
    {
        if(n1[i] >= n2[i])
        {
            result[i] = n1[i] - n2[i] + '0';
        }
        else
        {
            result[i] = (n1[i] + 10) - n2[i] + '0';
            SNumber::Borrow10(n1, i);
        }
    }

    SNumber ret;
    ret.numBefPoint = result;
    ret.ApplyExponent10(exponent);
    ret.Trim();
    return ret;
}

void SNumber::Borrow10(string& s, int pos)
{
    for(int i=pos-1; i>=0; --i)
    {
        if(s[i] == '0')
        {
            s[i] = '9';
        }
        else
        {
            s[i]--;
            return;
        }
    }
}

bool SNumber::EqualAbs(const SNumber& other) const
{
    return numBefPoint == other.numBefPoint && numAftPoint == other.numAftPoint;
}

bool SNumber::LessAbs(const SNumber& other) const
{
    SNumber upper = *this;
    SNumber oth = other;
    int maxBPlen = max(oth.numBefPoint.size(), numBefPoint.size());
    int maxAPlen = max(oth.numAftPoint.size(), numAftPoint.size());
    upper.numBefPoint = string(maxBPlen - numBefPoint.size(), '0') + upper.numBefPoint;
    oth.numBefPoint = string(maxBPlen - other.numBefPoint.size(), '0') + oth.numBefPoint;
    upper.numAftPoint += string(maxAPlen - numAftPoint.size(), '0');
    oth.numAftPoint += string(maxAPlen - other.numAftPoint.size(), '0');

    if(upper.numBefPoint < oth.numBefPoint)
        return true;
    if(upper.numBefPoint > oth.numBefPoint)
        return false;
    if(upper.numAftPoint < oth.numAftPoint)
        return true;
    return false;
}
