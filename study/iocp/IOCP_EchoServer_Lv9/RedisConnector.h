#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>
#include <memory>
#include <chrono>

class RedisConnector
{
public:
    RedisConnector() = default;
    ~RedisConnector() = default;

    void init(const std::string& host, const uint16_t port, const std::string& password = "", const uint32_t timeout = 0)
    {
        m_host = host;
        m_port = port;
        m_password = password;
        m_timeoutMs = (timeout == 0) ? 500 : timeout;
        m_lastError = ""; // 초기화
    }

    // 인자 없는 connect
    bool connect()
    {
        try {
            sw::redis::ConnectionOptions opts;
            opts.host = m_host;
            opts.port = m_port;
            opts.password = m_password;
            opts.connect_timeout = std::chrono::milliseconds(m_timeoutMs);

            sw::redis::ConnectionPoolOptions poolOpts;
            poolOpts.size = 10;

            m_redis = std::make_unique<sw::redis::Redis>(opts, poolOpts);

            if (m_redis->ping() == "PONG") {
                return true;
            }
        }
        catch (const sw::redis::Error& e) {
            // 에러 발생 시 메시지 저장
            m_lastError = e.what();
            m_redis.reset();
        }
        return false;
    }

    // 인자 있는 connect
    bool connect(const std::string& host, const uint16_t port, const std::string& password = "", const uint32_t timeout = 0)
    {
        init(host, port, password, timeout);
        return connect();
    }

    // 에러 메시지 반환 기능 추가
    std::string getErrorStr() const
    {
        return m_lastError;
    }

    std::optional<std::string> get(const std::string& key) {
        if (!m_redis) return std::nullopt;
        try {
            auto val = m_redis->get(key);
            if (val) return *val;
        }
        catch (...) {}
        return std::nullopt;
    }

    bool get(const std::string& key, std::string& outValue) {
        auto res = get(key);
        if (res.has_value()) {
            outValue = res.value();
            return true;
        }
        return false;
    }

    std::optional<std::string> hget(const std::string& key, const std::string& field) {
        if (!m_redis) return std::nullopt;

        try {
            // m_redis->get 대신 hget을 사용해야 합니다.
            auto val = m_redis->hget(key, field);

            if (val) {
                return *val;
            }
        }
        catch (const sw::redis::Error& e) {
            m_lastError = e.what();
            printf("[Redis Error] HGET 실패: %s\n", e.what());
        }
        catch (...) {
            m_lastError = "알 수 없는 에러";
        }

        return std::nullopt;
    }

    // 불린 반환형 (매니저에서 쓰기 편한 버전)
    bool hget(const std::string& key, const std::string& field, std::string& outValue) {
        auto res = hget(key, field);
        if (res.has_value()) {
            outValue = res.value();
            return true;
        }
        return false;
    }

private:
    std::string m_host;
    uint16_t    m_port = 6379;
    std::string m_password;
    uint32_t    m_timeoutMs = 500;
    std::string m_lastError; // 마지막 에러 저장용

    std::unique_ptr<sw::redis::Redis> m_redis;
};