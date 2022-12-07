/* 
Contains all necessary structs and functions related to adding userbase capabilities to a xnet server.

addon identifier:
    #define XNET_UB_ID 400

funcs:
    create_userbase()
    add_user(name, password, permission)
    delete_user(name)
    search_user(name)
    delete_userbase()

operation enum values:
    delete_user: 401
    add_user: 402
    
customization:
    use_sessions = true;
    use_userbase = true;
    use_permissions = true;
*/
typedef int make_iso_compilers_happy;