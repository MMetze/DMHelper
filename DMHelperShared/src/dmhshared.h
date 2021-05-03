#ifndef DMHSHARED_H
#define DMHSHARED_H


class DMHShared
{

public:
    DMHShared();

    enum DMH_Message_State
    {
        DMH_Message_State_Error = -1,
        DMH_Message_State_OK,
    };

    enum DMH_Message
    {
        DMH_Message_INVALID = -1,   // -1
        DMH_Message_ERROR,          //  0
        DMH_Message_dm_push,        //  1
        DMH_Message_dm_push_pull,   //  2
        DMH_Message_pl_pull,        //  3
        DMH_Message_pl_pull_push,   //  4
        DMH_Message_ssn_ass,        //  5
        DMH_Message_ssn_create,     //  6
        DMH_Message_ssn_isowner,    //  7
        DMH_Message_ssn_rename,     //  8
        DMH_Message_ssn_remove,     //  9
        DMH_Message_ssn_renew,      // 10
        DMH_Message_ssn_close,      // 11
        DMH_Message_ssn_members,    // 12
        DMH_Message_ssn_general,    // 13
        DMH_Message_file_push,      // 14
        DMH_Message_file_pull,      // 15
        DMH_Message_file_list,      // 16
        DMH_Message_file_exists,    // 17
        DMH_Message_usr_create,     // 18
        DMH_Message_msg_send,       // 19
        DMH_Message_msg_poll,       // 20
        DMH_Message_msg_ack,        // 21
        DMH_Message_usr_info,       // 22
    };
};

#endif // DMHSHARED_H
