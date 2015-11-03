#pragma once

namespace protocol
{
    
namespace auth
{
    
enum packetType
{
    notiPacket = 0,
    cmdPacket,
};
    
enum notiPacket
{
    notiError = 0,
};
    
enum cmdPacket
{
    cmdAuthServerPublicKey = 0,
    cmdShareSymmetricKey,
    cmdValidEmail,
    cmdSignUp,
    cmdSignIn,
    cmdTokenInfo,
    cmdRenewToken,
    cmdDeleteAccount,
};
    
} //end of namespace auth
    

}