#include "asymmetric_crypto.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"
#include "base/symmetric_crypto.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

#include <filesystem>
#include <memory>

namespace base
{

namespace rsa
{

    RsaPublicKey::RsaPublicKey(const base::Bytes& key_word)
        : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
    {}

    Bytes RsaPublicKey::encrypt(const Bytes& message) const
    {
        if(message.size() > maxEncryptSize()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes encrypted_message(encryptedMessageSize());
        if(!RSA_public_encrypt(message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(),
               RSA_PKCS1_OAEP_PADDING)) {
            RAISE_ERROR(CryptoError, "rsa encryption failed");
        }

        return encrypted_message;
    }

    Bytes RsaPublicKey::encryptWithtAes(const Bytes& message) const
    {
        base::aes::AesKey symmetric_key(base::aes::KeyType::Aes256BitKey);
        auto encrypted_message = symmetric_key.encrypt(message);
        auto serialized_symmetric_key = symmetric_key.toBytes();
        auto encrypted_serialized_symmetric_key = encrypt(serialized_symmetric_key);

        Bytes encrypted_serialized_key_size(sizeof(std::uint_least32_t));
        std::uint_least32_t key_size = encrypted_serialized_symmetric_key.size();
        std::memcpy(encrypted_serialized_key_size.toArray(), &key_size, encrypted_serialized_key_size.size());

        return encrypted_serialized_key_size.append(encrypted_serialized_symmetric_key).append(encrypted_message);
    }

    Bytes RsaPublicKey::decrypt(const Bytes& encrypted_message) const
    {
        if(encrypted_message.size() != encryptedMessageSize()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes decrypted_message(encryptedMessageSize());
        auto message_size = RSA_public_decrypt(encrypted_message.size(), encrypted_message.toArray(),
            decrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING);
        if(message_size == -1) {
            RAISE_ERROR(CryptoError, "rsa decryption failed");
        }

        return decrypted_message.takePart(0, message_size);
    }

    std::size_t RsaPublicKey::maxEncryptSize() const noexcept
    {
        return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
    }

    Bytes RsaPublicKey::toBytes() const
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> public_bio(BIO_new(BIO_s_mem()), ::BIO_free);

        if(!PEM_write_bio_RSAPublicKey(public_bio.get(), _rsa_key.get())) {
            RAISE_ERROR(CryptoError, "failed to write public RSA key to big num");
        }

        Bytes public_key_bytes(BIO_pending(public_bio.get()));
        if(!BIO_read(public_bio.get(), public_key_bytes.toArray(), public_key_bytes.size())) {
            RAISE_ERROR(CryptoError, "failed to write big num with public RSA to bytes");
        }

        return public_key_bytes;
    }

    std::size_t RsaPublicKey::encryptedMessageSize() const noexcept
    {
        return _encrypted_message_size;
    }

    std::unique_ptr<RSA, decltype(&::RSA_free)> RsaPublicKey::loadKey(const Bytes& key_word)
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> bio(
            BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
        RSA* rsa_key = nullptr;
        if(!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, NULL, NULL)) {
            RAISE_ERROR(CryptoError, "Fail to read public RSA key");
        }
        return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
    }

    RsaPrivateKey::RsaPrivateKey(const base::Bytes& key_word)
        : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
    {}

    Bytes RsaPrivateKey::encrypt(const Bytes& message) const
    {
        if(message.size() > maxMessageSizeForEncrypt()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes encrypted_message(encryptedMessageSize());
        if(!RSA_private_encrypt(
               message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING)) {
            RAISE_ERROR(CryptoError, "rsa encryption failed");
        }

        return encrypted_message;
    }

    Bytes RsaPrivateKey::decrypt(const Bytes& encrypted_message) const
    {
        if(encrypted_message.size() != encryptedMessageSize()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes decrypt_message(encryptedMessageSize());
        auto message_size = RSA_private_decrypt(encrypted_message.size(), encrypted_message.toArray(),
            decrypt_message.toArray(), _rsa_key.get(), RSA_PKCS1_OAEP_PADDING);
        if(message_size == -1) {
            RAISE_ERROR(CryptoError, "rsa decryption failed");
        }
        return decrypt_message.takePart(0, message_size);
    }

    Bytes RsaPrivateKey::dectyptWithAes(const Bytes& message) const
    {
        Bytes encrypted_serialized_key_size = message.takePart(0, sizeof(std::uint_least32_t));
        std::uint_least32_t key_size = 0;
        std::memcpy(&key_size, encrypted_serialized_key_size.toArray(), encrypted_serialized_key_size.size());

        auto encrypted_serialized_symmetric_key =
            message.takePart(sizeof(std::uint_least32_t), key_size + sizeof(std::uint_least32_t));
        auto encrypted_message = message.takePart(key_size + sizeof(std::uint_least32_t), message.size());

        auto serialized_symmetric_key = decrypt(encrypted_serialized_symmetric_key);
        base::aes::AesKey symmetric_key(serialized_symmetric_key);

        return symmetric_key.decrypt(encrypted_message);
    }

    std::size_t RsaPrivateKey::maxMessageSizeForEncrypt() const noexcept
    {
        return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
    }

    Bytes RsaPrivateKey::toBytes() const
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> private_bio(BIO_new(BIO_s_mem()), ::BIO_free);

        if(!PEM_write_bio_RSAPrivateKey(private_bio.get(), _rsa_key.get(), NULL, NULL, 0, 0, NULL)) {
            RAISE_ERROR(CryptoError, "failed to write private RSA key to big num");
        }

        Bytes private_key_bytes(BIO_pending(private_bio.get()));
        if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
            RAISE_ERROR(CryptoError, "failed to write big num with private RSA to bytes");
        }

        return private_key_bytes;
    }

    std::size_t RsaPrivateKey::encryptedMessageSize() const noexcept
    {
        return _encrypted_message_size;
    }

    std::unique_ptr<RSA, decltype(&::RSA_free)> RsaPrivateKey::loadKey(const Bytes& key_word)
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> bio(
            BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
        RSA* rsa_key = nullptr;
        if(!PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_key, NULL, NULL)) {
            RAISE_ERROR(CryptoError, "Fail to read private RSA key");
        }
        return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
    }

    std::pair<RsaPublicKey, RsaPrivateKey> generateKeys(std::size_t keys_size)
    {
        // create big number for random generation
        std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
        if(!BN_set_word(bn.get(), RSA_F4)) {
            RAISE_ERROR(Error, "Fail to create big number for RSA generation");
        }
        // create rsa and fill by created big number
        std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
        if(!RSA_generate_key_ex(rsa.get(), keys_size, bn.get(), NULL)) {
            RAISE_ERROR(Error, "Fail to generate RSA key");
        }
        // ==================
        // create bio for public key
        std::unique_ptr<BIO, decltype(&::BIO_free)> public_bio(BIO_new(BIO_s_mem()), ::BIO_free);

        // get public key spec
        std::unique_ptr<RSA, decltype(&::RSA_free)> public_rsa_key(RSAPublicKey_dup(rsa.get()), ::RSA_free);


        // fill bio by public key spec
        if(!PEM_write_bio_RSAPublicKey(public_bio.get(), public_rsa_key.get())) {
            RAISE_ERROR(Error, "Fail to generate public RSA key");
        }

        // write rsa data
        Bytes public_key_bytes(BIO_pending(public_bio.get()));

        // check errors in generation
        if(!BIO_read(public_bio.get(), public_key_bytes.toArray(), public_key_bytes.size())) {
            RAISE_ERROR(Error, "Fail to check public RSA key");
        }
        // =============

        // create bio for private key
        std::unique_ptr<BIO, decltype(&::BIO_free)> private_bio(BIO_new(BIO_s_mem()), ::BIO_free);

        // get private key spec
        std::unique_ptr<RSA, decltype(&::RSA_free)> private_rsa_key(RSAPrivateKey_dup(rsa.get()), ::RSA_free);

        // fill bio by private key spec
        if(!PEM_write_bio_RSAPrivateKey(private_bio.get(), private_rsa_key.get(), NULL, NULL, 0, 0, NULL)) {
            RAISE_ERROR(Error, "Fail to generate private RSA key");
        }

        // write rsa data
        Bytes private_key_bytes(BIO_pending(private_bio.get()));

        // check errors in generation
        if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
            RAISE_ERROR(Error, "Fail to check private RSA key");
        }
        // =============

        return std::pair<RsaPublicKey, RsaPrivateKey>(public_key_bytes, private_key_bytes);
    }

} // namespace rsa

}; // namespace base
