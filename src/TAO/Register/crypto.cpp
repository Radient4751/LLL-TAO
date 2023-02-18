/*__________________________________________________________________________________________

        (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

        (c) Copyright The Nexus Developers 2014 - 2021

        Distributed under the MIT software license, see the accompanying
        file COPYING or http://www.opensource.org/licenses/mit-license.php.

        "act the way you'd like to be and soon you'll be the way you act" - Leonard Cohen

____________________________________________________________________________________________*/

#include <TAO/API/types/exception.h>

#include <TAO/Register/types/crypto.h>

/* Global TAO namespace. */
namespace TAO::Register
{
    /* Default constructor. */
    Crypto::Crypto()
    : Object    ()
    {
    }


    /* Copy Constructor. */
    Crypto::Crypto(const Crypto& rObject)
    : Object      (rObject)
    {
    }


    /* Move Constructor. */
    Crypto::Crypto(Crypto&& rObject) noexcept
    : Object      (std::move(rObject))
    {
    }


    /* Assignment operator overload */
    Crypto& Crypto::operator=(const Crypto& rObject)
    {
        vchState     = rObject.vchState;

        nVersion     = rObject.nVersion;
        nType        = rObject.nType;
        hashOwner    = rObject.hashOwner;
        nCreated     = rObject.nCreated;
        nModified    = rObject.nModified;
        hashChecksum = rObject.hashChecksum;

        nReadPos     = 0; //don't copy over read position
        mapData      = rObject.mapData;

        return *this;
    }


    /* Assignment operator overload */
    Crypto& Crypto::operator=(Crypto&& rObject) noexcept
    {
        vchState     = std::move(rObject.vchState);

        nVersion     = std::move(rObject.nVersion);
        nType        = std::move(rObject.nType);
        hashOwner    = std::move(rObject.hashOwner);
        nCreated     = std::move(rObject.nCreated);
        nModified    = std::move(rObject.nModified);
        hashChecksum = std::move(rObject.hashChecksum);

        nReadPos     = 0; //don't copy over read position
        mapData      = std::move(rObject.mapData);

        return *this;
    }


    /* Default Destructor */
    Crypto::~Crypto()
    {
    }


    /* Checks our credentials against a given generate key to make sure they match. */
    bool Crypto::CheckCredentials(const memory::encrypted_ptr<TAO::Ledger::Credentials>& pCredentials, const SecureString& strPIN)
    {
        /* Read the key type from crypto object register. */
        const uint256_t hashAuth =
            this->get<uint256_t>("auth");

        /* Check if the auth has is deactivated. */
        if(hashAuth == 0)
            throw TAO::API::Exception(-9, "auth key is disabled, please run profiles/create/auth to enable");

        /* Generate a key to check credentials against. */
        const uint256_t hashCheck =
            pCredentials->KeyHash("auth", 0, strPIN, hashAuth.GetType());

        /* Check for invalid authorization hash. */
        if(hashAuth != hashCheck)
            return false;

        return true;
    }


    /* Verify signature data to a pubkey and valid key hash inside crypto object register. */
    bool Crypto::VerifySignature(const std::string& strKey, const uint512_t& hashCheck, const bytes_t& vPubKey, const bytes_t& vSig)
    {
        return true;
    }


    /* Generate a signature from signature chain credentials with valid key hash. */
    bool Crypto::GenerateSignature(const std::string& strKey, const memory::encrypted_ptr<TAO::Ledger::Credentials>& pCredentials,
                                   const SecureString& strPIN, const uint512_t& hashCheck, bytes_t &vPubKey, bytes_t &vSig)
    {
        return true;
    }
}
