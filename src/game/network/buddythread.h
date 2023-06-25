/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Gamespy stuff
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"

typedef enum _GPEnum
{
    GP_ERROR = 0x0,
    GP_RECV_BUDDY_REQUEST = 0x1,
    GP_RECV_BUDDY_STATUS = 0x2,
    GP_RECV_BUDDY_MESSAGE = 0x3,
    GP_RECV_GAME_INVITE = 0x4,
    GP_TRANSFER_CALLBACK = 0x5,
    GP_INFO_CACHING = 0x100,
    GP_SIMULATION = 0x101,
    GP_BLOCKING = 0x1,
    GP_NON_BLOCKING = 0x0,
    GP_FIREWALL = 0x1,
    GP_NO_FIREWALL = 0x0,
    GP_CHECK_CACHE = 0x1,
    GP_DONT_CHECK_CACHE = 0x0,
    GP_VALID = 0x1,
    GP_INVALID = 0x0,
    GP_FATAL = 0x1,
    GP_NON_FATAL = 0x0,
    GP_MALE = 0x500,
    GP_FEMALE = 0x501,
    GP_PAT = 0x502,
    GP_MORE = 0x600,
    GP_DONE = 0x601,
    GP_NICK = 0x700,
    GP_EMAIL = 0x701,
    GP_PASSWORD = 0x702,
    GP_FIRSTNAME = 0x703,
    GP_LASTNAME = 0x704,
    GP_ICQUIN = 0x705,
    GP_HOMEPAGE = 0x706,
    GP_ZIPCODE = 0x707,
    GP_COUNTRYCODE = 0x708,
    GP_BIRTHDAY = 0x709,
    GP_SEX = 0x70a,
    GP_CPUBRANDID = 0x70b,
    GP_CPUSPEED = 0x70c,
    GP_MEMORY = 0x70d,
    GP_VIDEOCARD1STRING = 0x70e,
    GP_VIDEOCARD1RAM = 0x70f,
    GP_VIDEOCARD2STRING = 0x710,
    GP_VIDEOCARD2RAM = 0x711,
    GP_CONNECTIONID = 0x712,
    GP_CONNECTIONSPEED = 0x713,
    GP_HASNETWORK = 0x714,
    GP_OSSTRING = 0x715,
    GP_AIMNAME = 0x716,
    GP_PIC = 0x717,
    GP_OCCUPATIONID = 0x718,
    GP_INDUSTRYID = 0x719,
    GP_INCOMEID = 0x71a,
    GP_MARRIEDID = 0x71b,
    GP_CHILDCOUNT = 0x71c,
    GP_INTERESTS1 = 0x71d,
    GP_REPLACE = 0x1,
    GP_DONT_REPLACE = 0x0,
    GP_CONNECTED = 0x1,
    GP_NOT_CONNECTED = 0x0,
    GP_MASK_NONE = 0x0,
    GP_MASK_HOMEPAGE = 0x1,
    GP_MASK_ZIPCODE = 0x2,
    GP_MASK_COUNTRYCODE = 0x4,
    GP_MASK_BIRTHDAY = 0x8,
    GP_MASK_SEX = 0x10,
    GP_MASK_EMAIL = 0x20,
    GP_MASK_ALL = 0xff,
    GP_OFFLINE = 0x0,
    GP_ONLINE = 0x1,
    GP_PLAYING = 0x2,
    GP_STAGING = 0x3,
    GP_CHATTING = 0x4,
    GP_AWAY = 0x5,
    GP_INTEL = 0x1,
    GP_AMD = 0x2,
    GP_CYRIX = 0x3,
    GP_MOTOROLA = 0x4,
    GP_ALPHA = 0x5,
    GP_MODEM = 0x1,
    GP_ISDN = 0x2,
    GP_CABLEMODEM = 0x3,
    GP_DSL = 0x4,
    GP_SATELLITE = 0x5,
    GP_ETHERNET = 0x6,
    GP_WIRELESS = 0x7,
    GP_TRANSFER_SEND_REQUEST = 0x800,
    GP_TRANSFER_ACCEPTED = 0x801,
    GP_TRANSFER_REJECTED = 0x802,
    GP_TRANSFER_NOT_ACCEPTING = 0x803,
    GP_TRANSFER_NO_CONNECTION = 0x804,
    GP_TRANSFER_DONE = 0x805,
    GP_TRANSFER_CANCELLED = 0x806,
    GP_TRANSFER_LOST_CONNECTION = 0x807,
    GP_TRANSFER_ERROR = 0x808,
    GP_TRANSFER_THROTTLE = 0x809,
    GP_FILE_BEGIN = 0x80a,
    GP_FILE_PROGRESS = 0x80b,
    GP_FILE_END = 0x80c,
    GP_FILE_DIRECTORY = 0x80d,
    GP_FILE_SKIP = 0x80e,
    GP_FILE_FAILED = 0x80f,
    GP_FILE_READ_ERROR = 0x900,
    GP_FILE_WRITE_ERROR = 0x901,
    GP_FILE_DATA_ERROR = 0x902,
    GP_TRANSFER_SENDER = 0xa00,
    GP_TRANSFER_RECEIVER = 0xa01,
} GPEnum;

class BuddyRequest
{
public:
    enum
    {
        BUDDYREQUEST_LOGIN = 0x0,
        BUDDYREQUEST_RELOGIN = 0x1,
        BUDDYREQUEST_LOGOUT = 0x2,
        BUDDYREQUEST_MESSAGE = 0x3,
        BUDDYREQUEST_LOGINNEW = 0x4,
        BUDDYREQUEST_ADDBUDDY = 0x5,
        BUDDYREQUEST_DELBUDDY = 0x6,
        BUDDYREQUEST_OKADD = 0x7,
        BUDDYREQUEST_DENYADD = 0x8,
        BUDDYREQUEST_SETSTATUS = 0x9,
        BUDDYREQUEST_MAX = 0xa,
    } m_budyRequestType;

    union
    {
        struct
        {
            int recipient;
            unichar_t text[128];
        } message;
        struct
        {
            char nick[31];
            char email[51];
            char password[31];
            bool has_firewall;
        } login;
        struct
        {
            int id;
            unichar_t text[128];
        } add_buddy;
        struct
        {
            int id;
        } profile;
        struct
        {
            GPEnum status;
            char status_string[256];
            char location_string[256];
        } status;
    } m_arg;
};

class BuddyResponse;

class GameSpyBuddyMessageQueueInterface
{
public:
    virtual ~GameSpyBuddyMessageQueueInterface();
    virtual void Start_Thread() = 0;
    virtual void End_Thread() = 0;
    virtual bool Is_Thread_Running() = 0;
    virtual bool Is_Connected() = 0;
    virtual bool Is_Connecting() = 0;
    virtual void Add_Request(const BuddyRequest &request) = 0;
    virtual bool Get_Request(BuddyRequest &request) = 0;
    virtual void Add_Response(const BuddyResponse &response) = 0;
    virtual bool Get_Response(BuddyResponse &response) = 0;
    virtual int Get_Local_Profile_ID() = 0;
};

#ifdef GAME_DLL
extern GameSpyBuddyMessageQueueInterface *&g_theGameSpyBuddyMessageQueue;
#else
extern GameSpyBuddyMessageQueueInterface *g_theGameSpyBuddyMessageQueue;
#endif
