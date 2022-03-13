/*__________________________________________________________________________________________

			(c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

			(c) Copyright The Nexus Developers 2014 - 2019

			Distributed under the MIT software license, see the accompanying
			file COPYING or http://www.opensource.org/licenses/mit-license.php.

			"ad vocem populi" - To The Voice of The People

____________________________________________________________________________________________*/

#pragma once

#include <LLC/types/uint1024.h>

#include <TAO/Ledger/types/sigchain.h>
#include <TAO/Ledger/types/pinunlock.h>

#include <Util/include/json.h>
#include <Util/include/mutex.h>
#include <Util/include/memory.h>

/* Global TAO namespace. */
namespace TAO::API
{
    /** @class
     *
     *  This class is responsible for handling all authentication requests to verify credentials against sessions.
     *
     **/
    class Authentication
    {
    public:

        /* Enum to track default session indexes. */
        enum SESSION : uint8_t
        {
            DEFAULT  = 0,
            MINER    = 1,
        };


        /** @class
         *
         *  Class to hold session related data for quick access.
         *
         **/
        class Session
        {
            /** Our active sigchain object. **/
            memory::encrypted_ptr<TAO::Ledger::SignatureChain> pCredentials;


            /** Our active pin unlock object. **/
            memory::encrypted_ptr<TAO::Ledger::PinUnlock> pUnlock;


            /** Cache of the genesis-id for this session. **/
            uint256_t hashGenesis;


            /** Cache of current session type. **/
            uint8_t nType;


        public:

            /* Enum to track session type. */
            enum : uint8_t
            {
                EMPTY   = 0,
                LOCAL   = 1,
                REMOTE  = 2,
                NETWORK = 3,
            };


            /** Track failed authentication attempts. **/
            mutable std::atomic<uint64_t> nAuthFailures;


            /** Track the last time session was active. **/
            mutable std::atomic<uint64_t> nLastActive;


            /** Internal mutex for creating new transactions. **/
            mutable std::recursive_mutex CREATE_MUTEX;


            /** Default Constructor. **/
            Session()
            : pCredentials  (nullptr)
            , pUnlock       (nullptr)
            , hashGenesis   (0)
            , nType         (LOCAL)
            , nAuthFailures (0)
            , nLastActive   (runtime::unifiedtimestamp())
            , CREATE_MUTEX  ( )
            {
            }


            /** Copy Constructor. **/
            Session(const Session& rSession) = delete;


            /** Move Constructor. **/
            Session(Session&& rSession)
            : pCredentials  (std::move(rSession.pCredentials))
            , pUnlock       (std::move(rSession.pUnlock))
            , hashGenesis   (std::move(rSession.hashGenesis))
            , nType         (std::move(rSession.nType))
            , nAuthFailures (rSession.nAuthFailures.load())
            , nLastActive   (rSession.nLastActive.load())
            , CREATE_MUTEX  ( )
            {
                /* We wnat to reset our pointers here so they don't get sniped. */
                rSession.pCredentials.SetNull();
                rSession.pUnlock.SetNull();
            }


            /** Copy Assignment. **/
            Session& operator=(const Session& rSession) = delete;


            /** Move Assignment. **/
            Session& operator=(Session&& rSession)
            {
                pCredentials  = std::move(rSession.pCredentials);
                pUnlock       = std::move(rSession.pUnlock);
                hashGenesis   = std::move(rSession.hashGenesis);
                nType         = std::move(rSession.nType);
                nAuthFailures = rSession.nAuthFailures.load();
                nLastActive   = rSession.nLastActive.load();

                /* We wnat to reset our pointers here so they don't get sniped. */
                rSession.pCredentials.SetNull();
                rSession.pUnlock.SetNull();

                return *this;
            }


            /** Constructor based on geneis. **/
            Session(const SecureString& strUsername, const SecureString& strPassword, const uint8_t nTypeIn = LOCAL)
            : pCredentials  (new TAO::Ledger::SignatureChain(strUsername, strPassword))
            , pUnlock       (nullptr)
            , hashGenesis   (pCredentials->Genesis())
            , nType         (nTypeIn)
            , nAuthFailures (0)
            , nLastActive   (runtime::unifiedtimestamp())
            , CREATE_MUTEX  ( )
            {
            }


            /** Default Destructor. **/
            ~Session()
            {
                RECURSIVE(CREATE_MUTEX); //TODO: this lock should wait if session is being used to build a tx.

                /* Cleanup the credentials object. */
                if(!pCredentials.IsNull())
                    pCredentials.free();

                /* Cleanup the pin unlock object. */
                if(!pUnlock.IsNull())
                    pUnlock.free();
            }


            /** Credentials
             *
             *  Retrieve the active credentials from the current session.
             *
             *  @return The signature chain credentials.
             *
             **/
            const memory::encrypted_ptr<TAO::Ledger::SignatureChain>& Credentials() const
            {
                return pCredentials;
            }


            /** Unlock
             *
             *  Unlock and get the active pin from current session.
             *
             *  @param[out] strPIN The pin number to return by reference
             *  @param[in] nRequestedActions The actions requested for PIN unlock.
             *
             *  @return The secure string containing active pin.
             *
             **/
            bool Unlock(SecureString &strPIN, const uint8_t nRequestedActions) const
            {
                /* Check that we have initialized our PIN. */
                if(pUnlock.IsNull())
                    return false;

                /* Check against our unlock actions. */
                if(pUnlock->UnlockedActions() & nRequestedActions)
                {
                    /* Get a copy of our secure string. */
                    strPIN = pUnlock->PIN();
                    if(strPIN.empty())
                        return false;

                    return true;
                }

                return false;
            }


            /** Genesis
             *
             *  Get the genesis-id of the current session.
             *
             *  @return the genesis-id of logged in session.
             *
             **/
            uint256_t Genesis() const
            {
                return hashGenesis;
            }


            /** Type
             *
             *  Get the current session type
             *
             *  @return the genesis-id of logged in session.
             *
             **/
            uint8_t Type() const
            {
                return nType;
            }


            /** Active
             *
             *  Get the number of seconds a session has been active.
             *
             *  @return The number of seconds session has been active.
             *
             **/
            uint64_t Active() const
            {
                return (runtime::unifiedtimestamp() - nLastActive.load());
            }
        };


        /** Initialize
         *
         *  Initializes the current authentication systems.
         *
         **/
        static void Initialize();


        /** Insert
         *
         *  Insert a new session into authentication map.
         *
         *  @param[in] hashSession The session identifier to add by index.
         *  @param[in] rSession The session to add to the map.
         *
         **/
        static void Insert(const uint256_t& hashSession, Session& rSession);


        /** Active
         *
         *  Check if user is already authenticated by genesis-id and return the session.
         *
         *  @param[in] hashGenesis The current genesis-id to lookup for.
         *  @param[out] hashSession The current session-id if logged in.
         *
         *  @return true if session is authenticated.
         *
         **/
        static bool Active(const uint256_t& hashGenesis, uint256_t &hashSession);


        /** Lock
         *
         *  Lock a session by session-id by modulus for hashmap value.
         *  It's safe to lock by session-id, because we don't allow login from same genesis to two sessions.
         *
         *  Hashmap collisions results in queueing of two sessions per lock.
         *  Hashmap buckets is set by commandline argument -sessionlocks=N
         *
         *  @param[in] jParams The incoming json parameters to get session-id.
         *
         *  @return reference of lock in internal lock hashmap.
         *
         **/
        static std::recursive_mutex& Lock(const encoding::json& jParams);


        /** Caller
         *
         *  Get the genesis-id of the given caller using session from params.
         *
         *  @param[in] jParams The incoming parameters to parse session from.
         *  @param[out] hashCaller The genesis-id of the caller
         *
         *  @return True if the caller was found
         *
         **/
        static bool Caller(const encoding::json& jParams, uint256_t &hashCaller);


        /** Instance
         *
         *  Get an instance of current session indexed by session-id.
         *  This will throw if session not found, do not use without checking first.
         *
         *  @param[in] jParams The incoming parameters.
         *
         *  @return The active session.
         *
         **/
        static const memory::encrypted_ptr<TAO::Ledger::SignatureChain>& Credentials(const encoding::json& jParams);
        //static Session& Instance(const encoding::json& jParams);


        /** Unlock
         *
         *  Unlock and get the active pin from current session.
         *
         *  @param[in] jParams The incoming paramters to parse
         *  @param[out] strPIN The pin number to return by reference
         *  @param[in] nRequestedActions The actions requested for PIN unlock.
         *
         *  @return True if this unlock action was successful.
         *
         **/
        static std::recursive_mutex& Unlock(const encoding::json& jParams, SecureString &strPIN,
                           const uint8_t nRequestedActions = TAO::Ledger::PinUnlock::TRANSACTIONS);


        /** Terminate
         *
         *  Terminate an active session by parameters.
         *
         *  @param[in] jParams The incoming parameters for lookup.
         *
         **/
        static void Terminate(const encoding::json& jParams);


        /** Shutdown
         *
         *  Shuts down the current authentication systems.
         *
         **/
        static void Shutdown();


    private:


        /** Mutex to lock around critical data. **/
        static std::recursive_mutex MUTEX;


        /** Map to hold session related data. **/
        static std::map<uint256_t, Session> mapSessions;


        /** Vector of our lock objects for session level locks. **/
        static std::vector<std::recursive_mutex> vLocks;


        /** terminate_session
         *
         *  Terminate an active session by parameters.
         *
         *  @param[in] hashSession The incoming session to terminate.
         *
         **/
        static void terminate_session(const uint256_t& hashSession);


        /** increment_failures
         *
         *  Increment the failure counter to deauthorize user after failed auth.
         *
         *  @param[in] hashSession The incoming session to terminate.
         *
         **/
        static void increment_failures(const uint256_t& hashSession);


        /** default_session
         *
         *  Checks for the correct session-id for single user mode.
         *
         **/
        __attribute__((const)) static uint256_t default_session();
    };
}
