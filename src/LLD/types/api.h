/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2019

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#pragma once

#include <LLC/types/uint1024.h>

#include <LLD/templates/sector.h>
#include <LLD/cache/binary_lru.h>
#include <LLD/keychain/hashmap.h>


namespace LLD
{

  /** LocalDB
   *
   *  Database class for storing local wallet transactions.
   *
   **/
    class APIDB : public SectorDatabase<BinaryHashMap, BinaryLRU>
    {

    public:

        /** The Database Constructor. To determine file location and the Bytes per Record. **/
        APIDB(const uint8_t nFlagsIn = FLAGS::CREATE | FLAGS::WRITE,
            const uint32_t nBucketsIn = 77773, const uint32_t nCacheIn = 1024 * 1024);


        /** Default Destructor **/
        virtual ~APIDB();


        /** WriteSession
         *
         *  Writes a session's access time to the database.
         *
         *  @param[in] hashAddress The register address.
         *  @param[in] nActive The timestamp this session was last active
         *
         *  @return True if a session exists in the localdb
         *
         **/
        bool WriteSession(const uint256_t& hashGenesis, const uint64_t nActive);


        /** ReadSession
         *
         *  Reads a session's access time to the database.
         *
         *  @param[in] hashAddress The register address.
         *  @param[out] nActive The timestamp this session was last active
         *
         *  @return True if a session exists in the localdb
         *
         **/
        bool ReadSession(const uint256_t& hashGenesis, uint64_t &nActive);

    };
}