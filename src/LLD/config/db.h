/*__________________________________________________________________________________________

            (c) Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

            (c) Copyright The Nexus Developers 2014 - 2019

            Distributed under the MIT software license, see the accompanying
            file COPYING or http://www.opensource.org/licenses/mit-license.php.

            "ad vocem populi" - To the Voice of the People

____________________________________________________________________________________________*/

#pragma once

#include <cstdint>
#include <string>

#include <Util/include/config.h>

namespace LLD::Config
{
    /** Structure to contain the configuration variables for a generic database object. **/
    class DB
    {
    public:

        /** The string to hold the database location. **/
        std::string DIRECTORY;


        /** The string to hold the database name. **/
        std::string NAME;


        /** The keychain level flags. **/
        uint8_t FLAGS;


        /** No default constructor. **/
        DB() = delete;


        /** Copy Constructor. **/
        DB(const DB& map)
        : DIRECTORY               (map.DIRECTORY)
        , NAME                    (map.NAME)
        , FLAGS                   (map.FLAGS)
        {
        }


        /** Move Constructor. **/
        DB(DB&& map)
        : DIRECTORY               (std::move(map.DIRECTORY))
        , NAME                    (std::move(map.NAME))
        , FLAGS                   (std::move(map.FLAGS))
        {
        }


        /** Copy Assignment **/
        DB& operator=(const DB& map)
        {
            DIRECTORY               = map.DIRECTORY;
            NAME                    = map.NAME;
            FLAGS                   = map.FLAGS;

            return *this;
        }


        /** Move Assignment **/
        DB& operator=(DB&& map)
        {
            DIRECTORY               = std::move(map.DIRECTORY);
            NAME                    = std::move(map.NAME);
            FLAGS                   = std::move(map.FLAGS);

            return *this;
        }


        /** Destructor. **/
        ~DB()
        {
        }


        /** Required Constructor. **/
        DB(const std::string& strName, const uint8_t nFlags)
        : DIRECTORY               (config::GetDataDir() + strName + "/")
        , NAME                    (strName)
        , FLAGS                   (nFlags)
        {
        }
    };
}
