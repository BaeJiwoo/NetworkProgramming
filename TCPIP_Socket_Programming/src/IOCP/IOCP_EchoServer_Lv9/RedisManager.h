#pragma once
#include "hiredis.h"
#include <string>
#include <thread>
#include <mutex>
#include "RedisTask.h"
#include "ErrorCode.h"
#include "RedisConnector.h"

#pragma comment(lib, "ws2_32.lib")

/*
* PacketManager에서 RedisTask를 생성하고 PushTask호출
* 그럼 WorkerThread가 RedisTask 분석하고 그에 맞춰 처리
* mTaskResponseQueue에 Response가 삽입된 것을 PacketManager가 확인함
*/

class RedisManager {
public:
	RedisManager() = default;
	~RedisManager() = default;

	bool Run(std::string ip_, UINT16 port_, const UINT32 threadCount_) {

		//if (this == nullptr) { printf("Manager is null!"); return false; }
		if (Connect(ip_, port_) == false) {
			printf("[Error] Redis Connect false");
			return false;
		}

		mIsTaskRun = true;

		for (UINT32 i = 0; i < threadCount_; i++) {
			mTaskThreads.emplace_back(std::thread([this]() {TaskProcessThread(); }));
		}
		printf("Redis Manager Running...");
	}

	void End() {
		mIsTaskRun = false;

		for (auto& e: mTaskThreads) {
			if (e.joinable()) {
				e.join();
			}
		}
	}

	void PushTask(RedisTask task_) {
		std::lock_guard<std::mutex> lock(mLock);
		mRequestQueue.push_back(task_);
	}

	RedisTask TakeResponseTask() {
		std::lock_guard<std::mutex> lock(mLock);

		if (mResponseQueue.empty()) {
			return RedisTask();
		}

		auto task = mResponseQueue.front();
		mResponseQueue.pop_front();

		return task;
	}


private:
	bool Connect(std::string ip_, uint16_t port_) {
		if (mRedisConnector.connect(ip_.c_str(), port_) == false)
		{
			printf("connect error : %s\n", mRedisConnector.getErrorStr().c_str());
			return false;
		}
		else
		{
			printf("connect success !!!\n");
		}

		return true;
	}

	void TaskProcessThread() {
		printf("Redis Thread Started...\n");

		while (mIsTaskRun) {
			bool isIdle = true;

			if (auto task = TakeRequestTask(); task.TaskID != RedisTaskID::INVALID) {
				isIdle = false;

				if (task.TaskID == RedisTaskID::REQUEST_LOGIN) {
					// 로그인 처리 쓰레드
					auto pRequest = (RedisLoginReq*)task.pData;
					RedisLoginRes bodyData;
					bodyData.Result = (UINT16)ERROR_CODE::LOGIN_USER_INVALID_PW;

					std::string value;
					if (mRedisConnector.hget(pRequest->UserID, "pw", value))//Redis 클래스 추가
					{
						bodyData.Result = (UINT16)ERROR_CODE::NONE;
						if (value.compare(pRequest->UserPW) == 0) {
							bodyData.Result = (UINT16)ERROR_CODE::NONE;
						}
					}
					else 
					{
						// 실패 시 이유 확인
						std::string err = mRedisConnector.getErrorStr();
						if (err.empty()) {
							printf("Key Not Found (Nil) in Redis\n");
						}
						else {
							printf("Actual Redis Error: %s\n", err.c_str());
						}
					}

					RedisTask resTask;
					resTask.UserIndex = task.UserIndex;
					resTask.TaskID = RedisTaskID::RESPONSE_LOGIN;
					resTask.DataSize = sizeof(RedisLoginRes);
					resTask.pData = new char[resTask.DataSize];
					CopyMemory(resTask.pData, (char*)&bodyData, resTask.DataSize);

					PushResponse(resTask);
				}

				task.Release();
			}

			if (isIdle) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		printf("Redis 스레드 종료\n");
	}

	RedisTask TakeRequestTask() {
		std::lock_guard<std::mutex> lock(mLock);

		if (mRequestQueue.empty()) {
			return RedisTask();
		}

		auto task = mRequestQueue.front();
		mRequestQueue.pop_front();

		return task;
	}

	void PushResponse(RedisTask task_) {
		std::lock_guard<std::mutex> lock(mLock);
		mResponseQueue.push_back(task_);
	}

	RedisConnector mRedisConnector;

	std::mutex mLock;

	std::deque<RedisTask> mResponseQueue;

	std::deque<RedisTask> mRequestQueue;

	std::vector<std::thread> mTaskThreads;

	bool mIsTaskRun = false;
};


