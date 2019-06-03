/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2019

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#include <LLC/include/random.h>

#include <LLD/include/global.h>

#include <TAO/Operation/include/enum.h>
#include <TAO/Operation/include/execute.h>
#include <TAO/Operation/include/enum.h>

#include <TAO/Operation/types/stream.h>

#include <TAO/Register/include/rollback.h>
#include <TAO/Register/include/create.h>

#include <TAO/Ledger/types/transaction.h>

#include <unit/catch2/catch.hpp>

TEST_CASE( "Trust Primitive Tests", "[operation]" )
{
    using namespace TAO::Register;
    using namespace TAO::Operation;

    //create a trust object register
    {
        uint256_t hashAddress = LLC::GetRand256();
        uint256_t hashGenesis = LLC::GetRand256();
        {
            //create the transaction object
            TAO::Ledger::Transaction tx;
            tx.hashGenesis = hashGenesis;
            tx.nSequence   = 0;
            tx.nTimestamp  = runtime::timestamp();

            //create object
            Object object = CreateTrust();
            object << std::string("testing") << uint8_t(TYPES::MUTABLE) << uint8_t(TYPES::UINT256_T) << uint256_t(555);

            //payload
            tx[0] << uint8_t(OP::CREATE) << hashAddress << uint8_t(REGISTER::OBJECT) << object.GetState();

            //generate the prestates and poststates
            REQUIRE(tx.Build());

            //commit to disk
            REQUIRE(Execute(tx[0], TAO::Ledger::FLAGS::BLOCK));

            //write the trust index
            REQUIRE(LLD::Register->IndexTrust(hashGenesis, hashAddress));
        }

        //create an operation stream to set values.
        {
            //create the mock transaction object
            TAO::Ledger::Transaction tx;
            tx.nTimestamp  = runtime::timestamp();
            tx.nSequence   = 0;
            tx.hashGenesis = hashGenesis;

            //build stream
            TAO::Operation::Stream stream;
            stream << std::string("testing") << uint8_t(OP::TYPES::UINT256_T) << uint256_t(293923982);

            //build transaction object
            tx[0] << uint8_t(OP::WRITE) << hashAddress << stream.Bytes();

            //run tests
            REQUIRE(tx.Build());

            //reset the streams
            REQUIRE(Execute(tx[0], TAO::Ledger::FLAGS::BLOCK));
        }

        {
            //check values all match
            TAO::Register::Object object;
            REQUIRE(LLD::Register->ReadTrust(hashGenesis, object));

            //parse
            REQUIRE(object.Parse());

            //check standards
            REQUIRE(object.Standard() == OBJECTS::TRUST);
            REQUIRE(object.Base()     == OBJECTS::ACCOUNT);

            //check values
            REQUIRE(object.get<uint64_t>("balance") == 0);
            REQUIRE(object.get<uint64_t>("trust")   == 0);
            REQUIRE(object.get<uint256_t>("token") == 0);
            REQUIRE(object.get<uint256_t>("testing") == 293923982);

            //write trust
            REQUIRE(object.Write("testing", uint256_t(1111)));
            REQUIRE(LLD::Register->WriteTrust(hashGenesis, object));
        }

        {
            //check values all match
            TAO::Register::Object object;
            REQUIRE(LLD::Register->ReadTrust(hashGenesis, object));

            //parse
            REQUIRE(object.Parse());

            //check standards
            REQUIRE(object.Standard() == OBJECTS::TRUST);
            REQUIRE(object.Base()     == OBJECTS::ACCOUNT);

            //check values
            REQUIRE(object.get<uint64_t>("balance") == 0);
            REQUIRE(object.get<uint64_t>("trust")   == 0);
            REQUIRE(object.get<uint256_t>("token") == 0);
            REQUIRE(object.get<uint256_t>("testing") == 1111);
        }
    }
}
