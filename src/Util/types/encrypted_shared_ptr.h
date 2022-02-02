/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2019

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#pragma once

#include <mutex>
#include <inttypes.h>

#include <Util/include/debug.h>

namespace util::atomic
{
	/** encrypted_shared_ptr
	 *
	 *  Pointer with member access protected with a mutex.
	 *
	 **/
	template<class TypeName>
	class encrypted_shared_ptr
	{
		/** lock_control
		 *
		 *  Track the current pointer references.
		 *
		 **/
		struct lock_control
		{
			/** Recursive mutex for locking encrypted_shared_ptr. **/
			std::recursive_mutex MUTEX;


			/** Reference counter for active copies. **/
		    std::atomic<uint32_t> nCount;


			/** Reference counter for active accesses. **/
			std::atomic<uint32_t> nAccesses;


			/** Default Constructor. **/
			lock_control ( )
			: MUTEX  	 ( )
			, nCount 	 (0)
			, nAccesses  (0)
			{
			}


			/** count
			 *
			 *  Access atomic with easier syntax.
			 *
			 **/
			uint32_t count() const
			{
				return nCount.load();
			}
		};


		/** lock_proxy
		 *
		 *  Temporary class that unlocks a mutex when outside of scope.
		 *  Useful for protecting member access to a raw pointer.
		 *
		 **/
		class lock_proxy
		{
			/** The internal locking mutex. **/
		    mutable lock_control* pLock;


			/** The pointer being locked. **/
			TypeName* pData;


		public:

			/** Basic constructor
			 *
			 *  Assign the pointer and reference to the mutex.
			 *
			 *  @param[in] pData The pointer to shallow copy
			 *  @param[in] MUTEX_IN The mutex reference
			 *
			 **/
			lock_proxy(TypeName* pDataIn, lock_control* pLockIn)
			: pLock 	(pLockIn)
			, pData 	(pDataIn)
			{
				/* Decrypt memory on first proxy. */
				if(pLock->nAccesses.load() == 0)
				{
					//pData->Encrypt();

					debug::warning(FUNCTION, "decrypting memory ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));
				}

				++pLock->nAccesses;

				debug::warning(FUNCTION, "increment accesses ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));
			}


			/** Destructor
			 *
			 *  Unlock the mutex.
			 *
			 **/
			~lock_proxy()
			{
				--pLock->nAccesses;

				debug::warning(FUNCTION, "decrement accesses ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));

				/* Encrypt memory again when ref count is 0. */
				if(pLock->nAccesses.load() == 0)
				{
					//pData->Encrypt(); //second call encrypts the data
					debug::warning(FUNCTION, "encrypting memory ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));
				}

				/* Unlock our mutex. */
				pLock->MUTEX.unlock();
			}


			/** Const Member Access Operator.
			 *
			 *  Access the memory of the raw pointer.
			 *
			 **/
			TypeName* operator->() const
			{
				/* Check for dereferencing nullptr. */
				if(pData == nullptr)
					throw debug::exception(FUNCTION, "dereferencing nullptr");

				return pData;
			}


			/** Member Access Operator.
			 *
			 *  Access the memory of the raw pointer.
			 *
			 **/
			TypeName* operator->()
			{
				/* Check for dereferencing nullptr. */
				if(pData == nullptr)
					throw debug::exception(FUNCTION, "dereferencing nullptr");

				return pData;
			}
		};


	    /** The internal locking mutex. **/
	    mutable lock_control* pLock;


	    /** The internal raw poitner. **/
	    TypeName* pData;


	public:

	    /** Default Constructor. **/
	    encrypted_shared_ptr()
	    : pLock  	(new lock_control())
	    , pData  	(nullptr)
	    {
	    }


	    /** Constructor for storing. **/
	    encrypted_shared_ptr(TypeName* pDataIn)
	    : pLock     (new lock_control())
	    , pData     (pDataIn)
	    {
			/* Only increase count when adding non null data. */
			if(pData)
				++pLock->nCount;
	    }


	    /** Copy Constructor. **/
	    encrypted_shared_ptr(const encrypted_shared_ptr<TypeName>& ptrIn)
	    : pLock     (ptrIn.pLock)
	    , pData     (ptrIn.pData)
	    {
			/* Only increase count when adding non null data. */
			if(pData)
	        	++pLock->nCount;
	    }


	    /** Move Constructor. **/
	    encrypted_shared_ptr(encrypted_shared_ptr<TypeName>&& ptrIn)
	    : pLock 	(std::move(ptrIn.pLock))
	    , pData 	(std::move(ptrIn.pData))
	    {
			/* Only increase count when adding non null data. */
			if(pData)
				++pLock->nCount;
	    }


	    /** Destructor. **/
	    ~encrypted_shared_ptr()
	    {
			/* Adjust our reference count. */
			if(pLock->count() > 0)
				--pLock->nCount;

			/* Cleanup pointer while holding scoped lock. */
			bool fDeleted = false;
			{
				RECURSIVE(pLock->MUTEX);
				fDeleted = cleanup();
			}

			/* Delete our lock object if empty. */
			if(fDeleted || pLock->count() == 0)
			{
				delete pLock;
				pLock = nullptr;
			}
	    }


	    /** Copy Assignment operator. **/
	    encrypted_shared_ptr& operator=(const encrypted_shared_ptr<TypeName>& ptrIn)
	    {
			RECURSIVE(pLock->MUTEX);

	        /* Shallow copy pointer and control block. */
	        pLock  = ptrIn.pLock;
	        pData  = ptrIn.pData;

	        /* Increase our reference count now. */
			if(pData)
	        	++pLock->nCount;

			return *this;
	    }


		/** Move Assignment operator. **/
		encrypted_shared_ptr& operator=(encrypted_shared_ptr<TypeName>&& ptrIn)
		{
			RECURSIVE(pLock->MUTEX);

			/* Adjust our reference count for current pointer. */
			if(pLock->count() > 0)
				--pLock->nCount;

			/* Shallow copy pointer and control block. */
			pLock  = std::move(ptrIn.pLock);
			pData  = std::move(ptrIn.pData);

			/* Increase our reference count now. */
			if(pData)
				++pLock->nCount;

			return *this;
		}


	    /** Assignment operator. **/
	    encrypted_shared_ptr& operator=(TypeName* pDataIn)
		{
			RECURSIVE(pLock->MUTEX);

			store(pDataIn);

			return *this;
		}


	    /** Equivilent operator.
	     *
	     *  @param[in] a The data type to compare to.
	     *
	     **/
	    bool operator==(const TypeName& pDataIn) const
	    {
			RECURSIVE(pLock->MUTEX);

	        return get() == pDataIn;
	    }


	    /** Equivilent operator.
	     *
	     *  @param[in] a The data type to compare to.
	     *
	     **/
	    bool operator==(const TypeName* pDataIn) const
	    {
	        RECURSIVE(pLock->MUTEX);

	        return pData == pDataIn;
	    }


	    /** Not equivilent operator.
	     *
	     *  @param[in] a The data type to compare to.
	     *
	     **/
	    bool operator!=(const TypeName* pDataIn) const
	    {
	        RECURSIVE(pLock->MUTEX);

	        return pData != pDataIn;
	    }

	    /** Not operator
	     *
	     *  Check if the pointer is nullptr.
	     *
	     **/
	    bool operator!(void) const
	    {
	        RECURSIVE(pLock->MUTEX);

	        return pData == nullptr;
	    }


		/** Member access overload
		 *
		 *  Allow encrypted_shared_ptr access like a normal pointer.
		 *
		 **/
		lock_proxy operator->() const
		{
			/* Lock our mutex before going forward. */
			pLock->MUTEX.lock();

			return lock_proxy(pData, pLock);
		}


	    /** Member access overload
	     *
	     *  Allow encrypted_shared_ptr access like a normal pointer.
	     *
	     **/
	    lock_proxy operator->()
	    {
	        /* Lock our mutex before going forward. */
	        pLock->MUTEX.lock();

	        return lock_proxy(pData, pLock);
	    }


	    /** dereference operator overload
	     *
	     *  Load the object from memory.
	     *
	     **/
	    TypeName operator*() const
	    {
			RECURSIVE(pLock->MUTEX);

			return get();
	    }

	private:

		/** get
		 *
		 *  Get a copy of the data by dereferencing pointer and handling encryption/decryption.
		 *
		 **/
		TypeName get() const
		{
			/* Check for dereferencing nullptr. */
			if(pData == nullptr)
				throw debug::exception(FUNCTION, "dereferencing nullptr");

			/* Decrypt the data. */
			if(pLock->nAccesses.load() == 0)
			{
				//pData->Encrypt();
				debug::warning(FUNCTION, "decrypting memory ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));
			}

			++pLock->nAccesses;
			debug::warning(FUNCTION, "increment accesses ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));

			/* Grab a copy of dereference. */
			const TypeName tData = *pData;

			/* Encrypt the data now. */
			--pLock->nAccesses;
			debug::warning(FUNCTION, "decrement accesses ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));

			if(pLock->nAccesses.load() == 0)
			{
				//pData->Encrypt();
				debug::warning(FUNCTION, "encrypting memory ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));
			}

			return tData;
		}


		/** cleanup
		 *
		 *  Clean up pointers if references are set to zero.
		 *
		 **/
		bool cleanup()
		{
			/* Delete if no more references. */
			if(pLock->count() == 0)
			{
				/* Cleanup the main raw pointer. */
				if(pData)
				{
					/* little spin-lock to make sure we don't clear when another thread is accessing */
					while(pLock->nAccesses.load() > 0);

					/* Decrypt the data. */
					if(pLock->nAccesses.load() == 0)
					{
						//pData->Encrypt();
						debug::warning(FUNCTION, "decrypting memory ", VARIABLE(pData), " | ", VARIABLE(pLock->nAccesses.load()));
					}

					/* Get our address for deletion. */
					TypeName* pAddress = pData;
					pData = nullptr;

					/* Delete the data now. */
					delete pAddress;

					return true;
				}
			}

			return false;
		}


		/** store
		 *
		 *  Store a new value in the encrypted pointer. Cleanup dangling pointers if empty references.
		 *
		 *  @param[in] pDataIn The pointer we are replacing in this encrypted_ptr.
		 *
		 **/
		void store(TypeName* pDataIn)
		{
			/* Check if data is set. */
			if(pDataIn)
			{
				/* Check if we have existing memory allocated. */
				if(pData)
				{
					/* Adjust our reference count. */
					if(pLock->count() > 0)
						--pLock->nCount;

					/* Attempt to cleanup old pointers. */
					cleanup();

					/* We need to make new lock control if there are still others using old pointer. */
					if(pLock->count() > 0)
						pLock = new lock_control();
				}

				/* Shallow copy of the data. */
				pData  = pDataIn;

				/* Encypt our data now. */
				//pData->Encrypt();

				/* Increase our reference count now. */
				++pLock->nCount;
			}
			else
			{
				/* Check if we have existing memory allocated. */
				if(pData)
				{
					/* Adjust our reference count. */
					if(pLock->count() > 0)
						--pLock->nCount;

					/* Cleanup our pointers. */
					cleanup();
				}

				pData = nullptr;
			}
		}
	};
}
