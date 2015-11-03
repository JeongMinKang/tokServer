#pragma once

class appCrypto
{
public:
    static int masterPassword(string& masterPwd);
    static int privateKey(string& priKey, const string& masterPwd);
    static int publicKey(string& pubKey);
    static int symmetricKey(string& symKey, const string& encSymKey, const string& priKey);
};
