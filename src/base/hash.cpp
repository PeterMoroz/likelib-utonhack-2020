#include "hash.hpp"

#include "base/assert.hpp"

#include <openssl/sha.h>

namespace base
{

Sha256::Sha256(const std::string& data) : _bytes(data)
{
    ASSERT(_bytes.size() == SHA256_DIGEST_LENGTH);
}

Sha256::Sha256(const Bytes& data) : _bytes(data)
{
    ASSERT(_bytes.size() == SHA256_DIGEST_LENGTH);
}

std::string Sha256::toHex() const
{
    return _bytes.toHex();
}


std::string Sha256::toString() const
{
    return _bytes.toString();
}


const base::Bytes& Sha256::getBytes() const noexcept
{
    return _bytes;
}


bool Sha256::operator==(const Sha256& another) const
{
    return toString() == another.toString();
}


bool Sha256::operator!=(const Sha256& another) const
{
    return toString() != another.toString();
}


Sha256 Sha256::compute(const base::Bytes& data)
{
    base::Bytes ret(SHA256_DIGEST_LENGTH);
    SHA256(data.toArray(), data.size(),
        reinterpret_cast<unsigned char*>(ret.toArray())); // reinterpret_cast is necessary if base::Byte changes
    ASSERT(ret.size() == SHA256_DIGEST_LENGTH);
    return Sha256(ret);
}


const Sha256& getNullSha256()
{
    static const Sha256 null_hash(Bytes(32));
    return null_hash;
}


std::string Sha1::toHex() const
{
    return _bytes.toHex();
}


std::string Sha1::toString() const
{
    return _bytes.toString();
}


const base::Bytes& Sha1::getBytes() const noexcept
{
    return _bytes;
}


bool Sha1::operator==(const Sha1& another) const
{
    return toString() == another.toString();
}


bool Sha1::operator!=(const Sha1& another) const
{
    return toString() != another.toString();
}


Sha1 Sha1::compute(const base::Bytes& data)
{
    base::Bytes ret(SHA_DIGEST_LENGTH);
    SHA1(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(ret.toArray()));
    ASSERT(ret.size() == SHA_DIGEST_LENGTH);
    return Sha1(ret);
}


Sha1::Sha1(const Bytes& another) : _bytes(another)
{
    ASSERT(_bytes.size() == SHA_DIGEST_LENGTH);
}

} // namespace base
