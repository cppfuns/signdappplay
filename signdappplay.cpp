#include "signdappplay.hpp"

void signdappplay::transfer(account_name from, account_name to, asset quantity, string memo)
{
    if (from == _self || to != _self) {
        return;
    }

    // don't do anything on transfers from our reference account
    if (from == N(signdappplay)) {
      return;
    }
    
    eosio_assert(quantity.symbol == CORE_SYMBOL, "only core token allowed"); //string_to_symbol(4, "EOS")
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");

    //memo "your_account_name-your_owner_public_key-your_active_public_key" 分隔符支持“-” “:” 空格
    //去掉memo前面的空格
    memo.erase(memo.begin(), find_if(memo.begin(), memo.end(), [](int ch) {
        return !isspace(ch);
    }));
    //去掉memo后面的空格
    memo.erase(find_if(memo.rbegin(), memo.rend(), [](int ch) {
        return !isspace(ch);
    }).base(), memo.end());

    eosio_assert(memo.length() == 120 || memo.length() == 66, "Malformed Memo (not right length)");
    const string account_string = memo.substr(0, 12);
    const account_name new_account_name = string_to_name(account_string.c_str());
    eosio_assert(memo[12] == ':' || memo[12] == '-' || memo[12] == ' ', "Malformed Memo [12] == : or - or space");

    const string owner_key_str = memo.substr(13, 53);
    string active_key_str;

    if(memo[66] == ':' || memo[66] == '-' || memo[66] == ' ') {
      // active key provided
      active_key_str = memo.substr(67, 53);
    } else {
      // active key is the same as owner
      active_key_str = owner_key_str;
    }
    
    const abieos::public_key owner_pubkey =
        abieos::string_to_public_key(owner_key_str);
    const abieos::public_key active_pubkey =
        abieos::string_to_public_key(active_key_str);

    array<char, 33> owner_pubkey_char;
    copy(owner_pubkey.data.begin(), owner_pubkey.data.end(),
         owner_pubkey_char.begin());

    array<char, 33> active_pubkey_char;
    copy(active_pubkey.data.begin(), active_pubkey.data.end(),
         active_pubkey_char.begin());

    signup_public_key owner_signup_pubkey = {
        .type = (unsigned_int)abieos::key_type::k1,
        .data = owner_pubkey_char,
    };

    signup_public_key active_signup_pubkey = {
        .type = (unsigned_int)abieos::key_type::k1,
        .data = active_pubkey_char,
    };

    key_weight owner_pubkey_weight = {
        .key = owner_signup_pubkey,
        .weight = 1
    };

    key_weight active_pubkey_weight = {
        .key = active_signup_pubkey,
        .weight = 1
    };

    authority owner = authority{
        .threshold = 1,
        .keys = {owner_pubkey_weight},
        .accounts = {},
        .waits = {}
    };

    authority active = authority{
        .threshold = 1,
        .keys = {active_pubkey_weight},
        .accounts = {},
        .waits = {}
    };

    newaccount new_account = newaccount{
        .creator = _self,
        .name = new_account_name,
        .owner = owner,
        .active = active
    };

    asset stake_net(1000, CORE_SYMBOL);
    asset stake_cpu(1000, CORE_SYMBOL);
    asset buy_ram = quantity - stake_net - stake_cpu;
    eosio_assert(buy_ram.amount > 0, "Not enough balance to buy ram");

    // create account
    action(
            permission_level{ _self, N(active) },
            N(eosio),
            N(newaccount),
            new_account
    ).send();
    // buy ram
    action(
            permission_level{ _self, N(active)},
            N(eosio),
            N(buyram),
            make_tuple(_self, new_account_name, buy_ram)
    ).send();
    // delegate and transfer cpu and net
    action(
            permission_level{ _self, N(active)},
            N(eosio),
            N(delegatebw),
            make_tuple(_self, new_account_name, stake_net, stake_cpu, true)
    ).send();
}
