#pragma once

struct authConfig : public config
{
    string masterPwd;
    string priKey;
    string pubKey;
};

typedef database<myDb> defDb;
//typedef database<mongoDb> defDb;

typedef dbCaller<defDb> appDb;

enum errType
{
    errAuthServerPublicKey = 0,
    errShareSymmetricKey,
    errValidEmail,
    errSignUp,
    errSignIn,
    errTokenInfo,
    errRenewToken,
    errDeleteAccount,
    errTypeMax,
};

enum durationTokenType
{
    oneHourToken = 0,
    oneDayToken,
    oneMonthToken,
    threeMonthToken,
    maxToken,
};

bool validDurationTokenType(int durationTokenType);
