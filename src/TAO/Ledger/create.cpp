/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2018

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#include <LLC/include/random.h>

#include <LLD/include/global.h>

#include <TAO/Ledger/include/create.h>
#include <TAO/Ledger/include/constants.h>
#include <TAO/Ledger/include/timelocks.h>

#include <TAO/Ledger/include/difficulty.h>
#include <TAO/Ledger/include/supply.h>
#include <TAO/Ledger/include/chainstate.h>

#include <TAO/Operation/include/enum.h>

namespace TAO::Ledger
{

    /*Create a new transaction object from signature chain.*/
    bool CreateTransaction(TAO::Ledger::SignatureChain* user, SecureString pin, TAO::Ledger::Transaction& tx)
    {
        /* Get the last transaction. */
        uint512_t hashLast;
        if(LLD::locDB->ReadLast(user->Genesis(), hashLast))
        {
            /* Get previous transaction */
            TAO::Ledger::Transaction txPrev;
            if(!LLD::legDB->ReadTx(hashLast, txPrev))
                return debug::error(FUNCTION "no prev tx in ledger db", __PRETTY_FUNCTION__);

            /* Build new transaction object. */
            tx.nSequence   = txPrev.nSequence + 1;
            tx.hashGenesis = txPrev.hashGenesis;
            tx.hashPrevTx  = hashLast;
            tx.NextHash(user->Generate(tx.nSequence + 1, pin.c_str()));

            return true;
        }

        /* Genesis Transaction. */
        tx.NextHash(user->Generate(tx.nSequence + 1, pin.c_str()));
        tx.hashGenesis = user->Genesis();

        return true;
    }


    /* Create a new block object from the chain.*/
    bool CreateBlock(TAO::Ledger::SignatureChain* user, SecureString pin, uint32_t nChannel, TAO::Ledger::TritiumBlock& block)
    {
        /* Set the block to null. */
        block.SetNull();


        /* Modulate the Block Versions if they correspond to their proper time stamp */
        if(runtime::UnifiedTimestamp() >= (config::fTestNet ?
            TESTNET_VERSION_TIMELOCK[TESTNET_BLOCK_CURRENT_VERSION - 2] :
            NETWORK_VERSION_TIMELOCK[NETWORK_BLOCK_CURRENT_VERSION - 2]))
            block.nVersion = config::fTestNet ?
            TESTNET_BLOCK_CURRENT_VERSION :
            NETWORK_BLOCK_CURRENT_VERSION; // --> New Block Versin Activation Switch
        else
            block.nVersion = config::fTestNet ?
            TESTNET_BLOCK_CURRENT_VERSION - 1 :
            NETWORK_BLOCK_CURRENT_VERSION - 1;


        /* Setup the producer transaction. */
        if(!CreateTransaction(user, pin, block.producer))
            return debug::error(FUNCTION "failed to create producer transactions", __PRETTY_FUNCTION__);


        /* Handle the coinstake */
        if (nChannel == 0)
        {
            if(block.producer.IsGenesis())
            {
                /* Create trust transaction. */
                block.producer << (uint8_t) TAO::Operation::OP::TRUST;

                /* The account that is being staked. */
                uint256_t hashAccount;
                block.producer << hashAccount;

                /* The previous trust block. */
                uint1024_t hashLastTrust = 0;
                block.producer << hashLastTrust;

                /* Previous trust sequence number. */
                uint32_t nSequence = 0;
                block.producer << nSequence;

                /* The previous trust calculated. */
                uint64_t nLastTrust = 0;
                block.producer << nLastTrust;

                /* The total to be staked. */
                uint64_t  nStake = 0; //account balance
                block.producer << nStake;
            }
            else
            {
                /* Create trust transaction. */
                block.producer << (uint8_t) TAO::Operation::OP::TRUST;

                /* The account that is being staked. */
                uint256_t hashAccount;
                block.producer << hashAccount;

                /* The previous trust block. */
                uint1024_t hashLastTrust = 0; //GET LAST TRUST BLOCK FROM LOCAL DB
                block.producer << hashLastTrust;

                /* Previous trust sequence number. */
                uint32_t nSequence = 0; //GET LAST SEQUENCE FROM LOCAL DB
                block.producer << nSequence;

                /* The previous trust calculated. */
                uint64_t nLastTrust = 0; //GET LAST TRUST FROM LOCAL DB
                block.producer << nLastTrust;

                /* The total to be staked. */
                uint64_t  nStake = 0; //BALANCE IS PREVIOUS BALANCE + INTEREST RATE
                block.producer << nStake;
            }
        }

        /** Create the Coinbase Transaction if the Channel specifies. **/
        else
        {
            /* Create coinbase transaction. */
            block.producer << (uint8_t) TAO::Operation::OP::COINBASE;

            /* The account that is being credited. */
            uint256_t hashAccount = LLC::GetRand256();
            block.producer << hashAccount;

            /* The total to be credited. */
            uint64_t  nCredit = GetCoinbaseReward(ChainState::stateBest, nChannel, 0);
            block.producer << nCredit;

        }

        /* Add the transactions. */
        std::vector<uint512_t> vHashes;
        vHashes.push_back(block.producer.GetHash());

        /** Populate the Block Data. **/
        block.hashPrevBlock   = ChainState::stateBest.GetHash();
        block.hashMerkleRoot = block.BuildMerkleTree(vHashes);
        block.nChannel       = nChannel;
        block.nHeight        = ChainState::stateBest.nHeight + 1;
        block.nBits          = GetNextTargetRequired(ChainState::stateBest, nChannel);
        block.nNonce         = 1;
        block.nTime          = runtime::UnifiedTimestamp();

        return true;
    }

}
