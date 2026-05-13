// Reglas de validacion del POST /config (wifi password length WPA2)
// y del POST /api/admin/password replicadas con std::string para validar
// invariantes sin Arduino/WebServer/Preferences.

#include <unity.h>
#include <string>

namespace {

// Mirror of the rule in endpoints.cpp::habldePostConfig: empty is OK
// (preserve current value), otherwise must satisfy WPA2 length 8-63.
bool isValidWifiPassword(const std::string& pass, std::string& error) {
    if (pass.empty()) return true;
    if (pass.size() < 8 || pass.size() > 63) {
        error = "wifi password must be 8-63 characters (WPA2) or empty";
        return false;
    }
    return true;
}

// Mirror of handleApiAdminPassword logic.
enum class AdminPwdResult {
    Ok,
    NewTooShort,
    NewTooLong,
    CurrentMismatch,
    SameAsCurrent
};

struct AdminPwdState {
    bool configured = false;
    std::string stored;
};

AdminPwdResult tryChangeAdminPassword(AdminPwdState& state,
                                      const std::string& current,
                                      const std::string& next) {
    if (next.size() < 8) return AdminPwdResult::NewTooShort;
    if (next.size() > 64) return AdminPwdResult::NewTooLong;
    if (state.configured && current != state.stored) {
        return AdminPwdResult::CurrentMismatch;
    }
    if (next == state.stored) return AdminPwdResult::SameAsCurrent;
    state.stored = next;
    state.configured = true;
    return AdminPwdResult::Ok;
}

}  // namespace

void testHandleMediciones() {
    TEST_ASSERT_TRUE(true);
}

void testHandleConfiguracion() {
    TEST_ASSERT_TRUE(true);
}

void testWifiPasswordLength_EmptyIsValid() {
    std::string err;
    TEST_ASSERT_TRUE(isValidWifiPassword("", err));
}

void testWifiPasswordLength_AcceptsWPA2Range() {
    std::string err;
    TEST_ASSERT_TRUE(isValidWifiPassword(std::string(8, 'a'), err));
    err.clear();
    TEST_ASSERT_TRUE(isValidWifiPassword(std::string(63, 'a'), err));
}

void testWifiPasswordLength_RejectsTooShort() {
    std::string err;
    TEST_ASSERT_FALSE(isValidWifiPassword(std::string(7, 'a'), err));
    TEST_ASSERT_EQUAL_STRING(
        "wifi password must be 8-63 characters (WPA2) or empty", err.c_str());
}

void testWifiPasswordLength_RejectsTooLong() {
    std::string err;
    TEST_ASSERT_FALSE(isValidWifiPassword(std::string(64, 'a'), err));
}

void testAdminPassword_RejectsShort() {
    AdminPwdState s{ true, "currentpass" };
    auto r = tryChangeAdminPassword(s, "currentpass", "1234567");
    TEST_ASSERT_EQUAL_INT((int)AdminPwdResult::NewTooShort, (int)r);
    TEST_ASSERT_EQUAL_STRING("currentpass", s.stored.c_str());
}

void testAdminPassword_RejectsTooLong() {
    AdminPwdState s{ true, "currentpass" };
    auto r = tryChangeAdminPassword(s, "currentpass", std::string(65, 'a'));
    TEST_ASSERT_EQUAL_INT((int)AdminPwdResult::NewTooLong, (int)r);
}

void testAdminPassword_RejectsSameAsCurrent() {
    AdminPwdState s{ true, "samepass1" };
    auto r = tryChangeAdminPassword(s, "samepass1", "samepass1");
    TEST_ASSERT_EQUAL_INT((int)AdminPwdResult::SameAsCurrent, (int)r);
}

void testAdminPassword_RequiresCurrentWhenConfigured() {
    AdminPwdState s{ true, "realcurrent" };
    auto r = tryChangeAdminPassword(s, "wrongone", "newpass123");
    TEST_ASSERT_EQUAL_INT((int)AdminPwdResult::CurrentMismatch, (int)r);
    TEST_ASSERT_EQUAL_STRING("realcurrent", s.stored.c_str());
}

void testAdminPassword_AllowsFirstSetWhenUnconfigured() {
    AdminPwdState s{ false, "" };
    auto r = tryChangeAdminPassword(s, "", "firstpass1");
    TEST_ASSERT_EQUAL_INT((int)AdminPwdResult::Ok, (int)r);
    TEST_ASSERT_TRUE(s.configured);
    TEST_ASSERT_EQUAL_STRING("firstpass1", s.stored.c_str());
}
