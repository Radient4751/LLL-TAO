/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2019

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#include <LLD/include/global.h>

#include <TAO/Operation/include/enum.h>
#include <TAO/Operation/types/contract.h>

#include <TAO/Register/include/enum.h>
#include <TAO/Register/include/rollback.h>
#include <TAO/Register/types/state.h>

/* Global TAO namespace. */
namespace TAO
{

    /* Register Layer namespace. */
    namespace Register
    {

        /* Verify the pre-states of a register to current network state. */
        bool Rollback(const TAO::Operation::Contract& contract)
        {
            /* Reset the contract streams. */
            contract.Reset();

            /* Make sure no exceptions are thrown. */
            try
            {
                /* Get the contract OP. */
                uint8_t nOP = 0;
                contract >> nOP;

                /* Check the current opcode. */
                switch(nOP)
                {

                    /* Condition that allows a validation to occur. */
                    case TAO::Operation::OP::CONDITION:
                    {
                        /* Condition has no parameters. */
                        contract >> nOP;

                        /* Condition has no parameters. */
                        break;
                    }


                    /* Validate a previous contract's conditions */
                    case TAO::Operation::OP::VALIDATE:
                    {
                        /* Extract the transaction from contract. */
                        uint512_t hashTx = 0;
                        contract >> hashTx;

                        /* Extract the contract-id. */
                        uint32_t nContract = 0;
                        contract >> nContract;

                        /* Erase the contract validation record. */
                        if(!LLD::Contract->EraseContract(std::make_pair(hashTx, nContract)))
                            return debug::error(FUNCTION, "failed to erase validation contract");

                        /* Condition has no parameters. */
                        contract >> nOP;

                        break;
                    }
                }

                /* Check the current opcode. */
                switch(nOP)
                {

                    /* Check pre-state to database. */
                    case TAO::Operation::OP::WRITE:
                    {
                        /* Get the Address of the Register. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Unpack the post-state vector. */
                        std::vector<uint8_t> vchState;
                        contract >> vchState;

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::WRITE: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashAddress, state))
                            return debug::error(FUNCTION, "OP::WRITE: failed to rollback to pre-state");

                        return true;
                    }


                    /* Check pre-state to database. */
                    case TAO::Operation::OP::APPEND:
                    {
                        /* Get the Address of the Register. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Unpack the post-state vector. */
                        std::vector<uint8_t> vchState;
                        contract >> vchState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::APPEND: egister state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashAddress, state))
                            return debug::error(FUNCTION, "OP::APPEND: failed to rollback to pre-state");

                        return true;
                    }


                    /* Delete the state on rollback. */
                    case TAO::Operation::OP::CREATE:
                    {
                        /* Get the address of the Register. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Unpack the register type. */
                        uint8_t nType = 0;
                        contract >> nType;

                        /* Unpack the post-state vector. */
                        std::vector<uint8_t> vchState;
                        contract >> vchState;

                        /* Erase the register from database. */
                        if(!LLD::Register->EraseState(hashAddress))
                            return debug::error(FUNCTION, "OP::CREATE: failed to erase post-state");

                        break;
                    }


                    /* Transfer ownership of a register to another signature chain. */
                    case TAO::Operation::OP::TRANSFER:
                    {
                        /* Get the Address of the Register. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Seek to end. */
                        contract.Seek(32);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::TRANSFER: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashAddress, state))
                            return debug::error(FUNCTION, "OP::TRANSFER: failed to rollback to pre-state");

                        break;
                    }


                    /* Transfer ownership of a register to another signature chain. */
                    case TAO::Operation::OP::CLAIM:
                    {
                        /* The transaction that this transfer is claiming. */
                        uint512_t hashTx = 0;
                        contract >> hashTx;

                        /* Get the contract number. */
                        uint32_t nContract = 0;
                        contract >> nContract;

                        /* Get the address to update. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Erase the proof. */
                        if(!LLD::Ledger->EraseProof(hashAddress, hashTx, nContract))
                            return debug::error(FUNCTION, "OP::CLAIM: failed to erase claim proof");

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::CLAIM: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashAddress, state))
                            return debug::error(FUNCTION, "OP::CLAIM: failed to rollback to pre-state");

                        break;
                    }


                    /* Coinbase operation. Creates an account if none exists. */
                    case TAO::Operation::OP::COINBASE:
                    {
                        /* Seek through coinbase data. */
                        contract.Seek(48);

                        break;
                    }


                    /* Coinstake operation. Requires an account. */
                    case TAO::Operation::OP::TRUST:
                    {
                        /* Seek to end. */
                        contract.Seek(80);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::TRUST: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteTrust(contract.Caller(), state))
                            return debug::error(FUNCTION, "OP::TRUST: failed to rollback to pre-state");

                        break;
                    }


                    /* Coinstake operation. Requires an account. */
                    case TAO::Operation::OP::GENESIS:
                    {
                        /* Get last trust block. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Seek to end. */
                        contract.Seek(8);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::GENESIS: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashAddress, state))
                            return debug::error(FUNCTION, "OP::GENESIS: failed to rollback to pre-state");

                        /* Erase the trust index. */
                        if(!LLD::Register->EraseTrust(contract.Caller()))
                            return debug::error(FUNCTION, "OP::GENESIS: failed to erase trust index");

                        break;
                    }


                    /* Add stake */
                    case TAO::Operation::OP::STAKE:
                    {
                        /* Seek to end. */
                        contract.Seek(8);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::STAKE: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteTrust(contract.Caller(), state))
                            return debug::error(FUNCTION, "OP::STAKE: failed to rollback to pre-state");

                        break;
                    }


                    /* Remove stake */
                    case TAO::Operation::OP::UNSTAKE:
                    {
                        /* Seek to end. */
                        contract.Seek(16);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::UNSTAKE: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteTrust(contract.Caller(), state))
                            return debug::error(FUNCTION, "OP::UNSTAKE: failed to rollback to pre-state");

                        break;
                    }


                    /* Debit tokens from an account you own. */
                    case TAO::Operation::OP::DEBIT:
                    {
                        /* Get last trust block. */
                        uint256_t hashFrom = 0;
                        contract >> hashFrom;

                        /* Get last trust block. */
                        uint256_t hashTo = 0;
                        contract >> hashTo;

                        /* Seek to end. */
                        contract.Seek(8);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::DEBIT: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashFrom, state))
                            return debug::error(FUNCTION, "OP::DEBIT: failed to rollback to pre-state");

                        /* Read the owner of register. */
                        TAO::Register::State stateTo;
                        if(!LLD::Register->ReadState(hashTo, stateTo))
                            return debug::error(FUNCTION, "failed to read register to");

                        /* Write the event to the ledger database. */
                        if(!LLD::Ledger->EraseEvent(stateTo.hashOwner))
                            return debug::error(FUNCTION, "OP::DEBIT: failed to rollback event");

                        break;
                    }


                    /* Credit tokens to an account you own. */
                    case TAO::Operation::OP::CREDIT:
                    {
                        /* Extract the transaction from contract. */
                        uint512_t hashTx = 0;
                        contract >> hashTx;

                        /* Extract the contract-id. */
                        uint32_t nContract = 0;
                        contract >> nContract;

                        /* Get the transfer address. */
                        uint256_t hashAddress = 0;
                        contract >> hashAddress;

                        /* Get the transfer address. */
                        uint256_t hashProof = 0;
                        contract >> hashProof;

                        /* Get the transfer amount. */
                        uint64_t  nAmount = 0;
                        contract >> nAmount;

                        /* Write the claimed proof. */
                        if(!LLD::Ledger->EraseProof(hashProof, hashTx, nContract))
                            return debug::error(FUNCTION, "OP::CREDIT: failed to erase credit proof");

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::CREDIT: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashAddress, state))
                            return debug::error(FUNCTION, "OP::CREDIT: failed to rollback to pre-state");

                        /* Read the debit. */
                        const TAO::Operation::Contract debit = LLD::Ledger->ReadContract(hashTx, nContract);
                        debit.Seek(1);

                        /* Get address from. */
                        uint256_t hashFrom = 0;
                        debit  >> hashFrom;

                        /* Check for partial credits. */
                        if(state.hashOwner != hashProof && hashFrom != hashProof)
                        {
                            /* Get the partial amount. */
                            uint64_t nClaimed = 0;
                            if(!LLD::Ledger->ReadClaimed(hashTx, nContract, nClaimed))
                                return debug::error(FUNCTION, "OP::CREDIT: failed to read claimed amount");

                            /* Sanity check for claimed overflow. */
                            if(nClaimed < nAmount)
                                return debug::error(FUNCTION, "OP::CREDIT: amount larger than claimed (overflow)");

                            /* Write the new claimed amount. */
                            if(!LLD::Ledger->WriteClaimed(hashTx, nContract, (nClaimed - nAmount)))
                                return debug::error(FUNCTION, "OP::CREDIT: failed to rollback claimed amount");
                        }

                        break;
                    }


                    /* Authorize is enabled in private mode only. */
                    case TAO::Operation::OP::AUTHORIZE:
                    {
                        /* Seek to address. */
                        contract.Seek(96);

                        break;
                    }


                    /* Debit tokens from an account you own. */
                    case TAO::Operation::OP::LEGACY:
                    {
                        /* Get account from block. */
                        uint256_t hashFrom = 0;
                        contract >> hashFrom;

                        /* Seek to end. */
                        contract.Seek(8);

                        /* Verify the first register code. */
                        uint8_t nState = 0;
                        contract >>= nState;

                        /* Check the state is prestate. */
                        if(nState != STATES::PRESTATE)
                            return debug::error(FUNCTION, "OP::LEGACY: register state not in pre-state");

                        /* Verify the register's prestate. */
                        State state;
                        contract >>= state;

                        /* Write the register from database. */
                        if(!LLD::Register->WriteState(hashFrom, state))
                            return debug::error(FUNCTION, "OP::LEGACY: failed to rollback to pre-state");

                        break;
                    }

                    default:
                        return debug::error(FUNCTION, "invalid code for contract execution");

                }

                /* Check for end of stream. */
                if(!contract.End())
                    return debug::error(FUNCTION, "can only have one PRIMITIVE per contract");
            }
            catch(const std::exception& e)
            {
                return debug::error(FUNCTION, "exception encountered ", e.what());
            }

            /* If nothing failed, return true for evaluation. */
            return true;
        }
    }
}
