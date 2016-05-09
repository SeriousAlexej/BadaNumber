#include "SNumber.h"
#include <stdexcept>

using namespace std;

ostream& operator<<(ostream& os, const SNumber& num)
{
    os << num.ToString();
    return os;
}

SNumber::SNumber(int value, int exp)
{
    if(value < 0)
    {
        negative = true;
        value *= -1;
    }
    numBefPoint = to_string(value);
    ApplyExponent10(exp);
    Trim();
}

SNumber::SNumber(const string& input)
{
    if(!Parse(input))
        (*this) = SNumber();
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
            return false;

        case R_EXPONENT:
            {
                int numExp;
                string::size_type sz;
                string leftover = input.substr(i);
                try
                {
                    numExp = stoi(leftover,&sz);
                }
                catch(...)
                {
                    return false;
                }
                if(sz != leftover.size())
                {
                    return false;
                }
                ApplyExponent10(numExp);
                i = lim;
                readPhase = R_DONE;
                break;
            }
        default:
            return false;
        }
    }
    if(readPhase != R_DONE)
    {
        return false;
    }
    Trim();
    if(IsZero())
    {
        negative = false;
    }
    return true;
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
            throw invalid_argument("Division by zero");
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

    int divisorABias=0, dividentABias=0;
    if(!divisor.numBefPoint.empty())
    {
        divisorABias = divisor.numBefPoint.size();
    }
    else
    {
        for(unsigned i=0; i<divisor.numAftPoint.size(); ++i)
        {
            if(divisor.numAftPoint[i] == '0')
                divisorABias--;
            else
                break;
        }
    }
    if(!divident.numBefPoint.empty())
    {
        dividentABias = divident.numBefPoint.size();
    }
    else if(!divident.numAftPoint.empty())
    {
        dividentABias = 0;
        for(unsigned i=0; i<divident.numAftPoint.size(); ++i)
        {
            if(divident.numAftPoint[i] == '0')
                dividentABias--;
            else
                break;
        }
    }

    int a = max(0, dividentABias - divisorABias - 1);
    SNumber numWhole = SNumber(1, a);
    SNumber tmp = divisor;
    tmp.ApplyExponent10(a);
    tmp.Trim();
    while(a >= 0)
    {
        SNumber tmp2 = divisor;
        tmp2.ApplyExponent10(a);
        tmp2.Trim();
        tmp2 += tmp;
        if(tmp2 > divident)
        {
            a--;
            continue;
        }
        tmp = tmp2;
        numWhole += SNumber(1, a);
    }
    if(tmp > divident)
    {
        tmp -= divisor;
        numWhole -= SNumber(1);
    }

    quotient.numBefPoint = numWhole.numBefPoint;

    divident = divident - tmp;
    divident.ApplyExponent10(1);
    divident.Trim();

    const int maxDigits = max(max(200, (int)divident.numAftPoint.size()), (int)divisor.numAftPoint.size());
    for(int i=0; i<maxDigits; ++i)
    {

        if(!divident.numBefPoint.empty())
        {
            dividentABias = divident.numBefPoint.size();
        }
        else if(!divident.numAftPoint.empty())
        {
            dividentABias = 0;
            for(unsigned i=0; i<divident.numAftPoint.size(); ++i)
            {
                if(divident.numAftPoint[i] == '0')
                    dividentABias--;
                else
                    break;
            }
        }
        else
        {
            break;
        }
        a = max(0, dividentABias - divisorABias - 1);
        numWhole = SNumber(1, a);
        tmp = divisor;
        tmp.ApplyExponent10(a);
        tmp.Trim();
        while(a >= 0)
        {
            SNumber tmp2 = divisor;
            tmp2.ApplyExponent10(a);
            tmp2.Trim();
            tmp2 += tmp;
            if(tmp2 > divident)
            {
                a--;
                continue;
            }
            tmp = tmp2;
            numWhole += SNumber(1, a);
        }
        if(tmp > divident)
        {
            tmp -= divisor;
            numWhole -= SNumber(1);
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

SNumber SNumber::operator^(int power) const
{
    SNumber one(1);
    if(power > 0)
        for(int i=0; i<power; ++i)
        {
            one *= (*this);
        }
    else if(power < 0)
        for(int i=0; i<-power; ++i)
        {
            one /= (*this);
        }
    return one;
}

SNumber& SNumber::operator^=(int power)
{
    (*this) = (*this)^power;
    return (*this);
}

SNumber SNumber::operator-(const SNumber& other) const
{
    SNumber minOther = other;
    minOther.negative = !minOther.negative;
    return (*this) + minOther;
}

SNumber SNumber::operator-() const
{
    SNumber opp = (*this);
    opp.negative = !negative;
    return opp;
}

SNumber& SNumber::operator+()
{
    return *this;
}

SNumber& SNumber::operator--()
{
    (*this) -= SNumber(1);
    return (*this);
}

SNumber SNumber::operator--(int)
{
    SNumber backup = (*this);
    (*this) -= SNumber(1);
    return backup;
}

SNumber& SNumber::operator++()
{
    (*this) += SNumber(1);
    return (*this);
}

SNumber SNumber::operator++(int)
{
    SNumber backup = (*this);
    (*this) += SNumber(1);
    return backup;
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
