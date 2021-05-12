/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2019

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#pragma once

#include <LLC/types/uint1024.h>

#include <Util/include/json.h>

#include <string.h>

/* Forward declarations */
namespace TAO::Register { class Object; }


/* Global TAO namespace. */
namespace TAO::API
{

    /** CheckAddress
     *
     *  Determines whether a Base58 encoded value is a valid register address by checking type.
     *
     *  @param[in] strValue The value to check
     *
     *  @return True if the value can be decoded into a valid Register::Address with a valid Type.
     **/
    bool CheckAddress(const std::string& strValue);


    /** CheckMature
     *
     *  Utilty method that checks that the signature chain is mature and can therefore create new transactions.
     *  Throws an appropriate APIException if it is not mature
     *
     *  @param[in] hashGenesis The genesis hash of the signature chain to check
     *
     **/
    void CheckMature(const uint256_t& hashGenesis);


    /** CheckContract
     *
     *  Checks if a contract will execute correctly once built.
     *
     *  @param[in] tContract The contract to check and sanitize.
     *
     *  @return true if the contract executed correctly.
     *
     **/
    bool CheckContract(const TAO::Operation::Contract& tContract);


    /** CheckType
     *
     *  Checks if the designated object matches the explicet type specified in parameters.
     *  We have no return value since this command is meant to throw on errors for API calls.
     *
     *  @param[in] params The json parameters to check against.
     *  @param[in] hashCheck The register that we are checking against.
     *
     *  @return True if the object type is what was specified.
     *
     **/
    void CheckType(const json::json& params, const uint256_t& hashCheck);


    /** CheckType
     *
     *  Checks if the designated object matches the explicet type specified in parameters.
     *  Doesn't do a register database lookup like prior overload does.
     *
     *  @param[in] params The json parameters to check against.
     *  @param[in] objCheck The object that we are checking for.
     *
     *  @return True if the object type is what was specified.
     *
     **/
    void CheckType(const json::json& params, const TAO::Register::Object& objCheck);


    /** CheckTypes
     *
     *  Checks if the designated object matches the explicet type specified in parameters.
     *  For use in list commands that check for 'accounts' or 'tokens'
     *
     *  @param[in] params The json parameters to check against.
     *  @param[in] objCheck The object that we are checking for.
     *
     *  @return True if the object type is what was specified.
     *
     **/
    bool CheckTypes(const json::json& params, const TAO::Register::Object& objCheck);
}
