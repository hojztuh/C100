#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

struct Header {
    int Length;
    int cmd;
};

struct Login: public Header {

    Login() {
        Length = sizeof(Login);
        cmd = CMD_LOGIN;
    }

    char Name[32];
    char Password[32];
    char data[32];
};

struct LoginResult: public Header {
    LoginResult() {
        Length = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
    }

    int result;
};

struct Logout: public Header {

    Logout() {
        Length = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }

    char Name[32];
};

struct LogoutResult: public Header {
    
    LogoutResult() {
        Length = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
    }

    int result;
};

struct NewUserJoin:public Header {

    NewUserJoin() {
        Length = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }

    int sock;

};

#endif