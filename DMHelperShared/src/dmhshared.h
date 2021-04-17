#ifndef DMHSHARED_H
#define DMHSHARED_H


class DMHShared
{

public:
    DMHShared();

    enum DMH_Message
    {
        DMH_Message_INVALID = 0,
        DMH_Message_dm_push,
        DMH_Message_dm_push_pull,
        DMH_Message_pl_pull,
        DMH_Message_pl_pull_push,
        DMH_Message_ssn_ass,
        DMH_Message_ssn_create,
        DMH_Message_ssn_isowner,
        DMH_Message_ssn_rename,
        DMH_Message_ssn_remove,
        DMH_Message_ssn_renew,
        DMH_Message_ssn_close,
        DMH_Message_ssn_members,
        DMH_Message_ssn_general,
        DMH_Message_file_push,
        DMH_Message_file_pull,
        DMH_Message_file_list,
        DMH_Message_file_exists,
        DMH_Message_usr_create
    };


};

#endif // DMHSHARED_H
