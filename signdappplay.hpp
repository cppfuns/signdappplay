#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/public_key.hpp>
#include "includes/abieos_numeric.hpp"
using namespace eosio;
using namespace std;
class signdappplay: public contract {
public:
    signdappplay(account_name self): contract(self){};
    void transfer(account_name from, account_name to, asset quantity, string memo);

private:
    struct permission_level_weight {
        permission_level permission;
        weight_type weight;
        EOSLIB_SERIALIZE(permission_level_weight, (permission)(weight))
    };

    struct key_weight {
        eosio::public_key key;
        weight_type weight;

        EOSLIB_SERIALIZE(key_weight, (key)(weight))
    };

    struct wait_weight {
        uint32_t wait_sec;
        weight_type weight;

        EOSLIB_SERIALIZE(wait_weight, (wait_sec)(weight))
    };

    struct authority {
        uint32_t threshold;
        vector<key_weight> keys;
        vector<permission_level_weight> accounts;
        vector<wait_weight> waits;

        EOSLIB_SERIALIZE(authority, (threshold)(keys)(accounts)(waits))
    };

    struct newaccount {
        account_name creator;
        account_name name;
        authority owner;
        authority active;

        EOSLIB_SERIALIZE(newaccount, (creator)(name)(owner)(active))
    };
};


#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
        auto self = receiver; \
        if( action == N(onerror)) { \
            /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
            eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
        } \
        if((code == N(eosio.token) && action == N(transfer)) ) { \
            TYPE thiscontract( self ); \
            switch( action ) { \
                EOSIO_API( TYPE, MEMBERS ) \
            } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
        } \
    } \
} \

EOSIO_ABI_EX(signdappplay, (transfer))