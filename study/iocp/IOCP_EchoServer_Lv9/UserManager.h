#pragma once
#include <unordered_map>
#include "User.h";
#include "ErrorCode.h"

class UserManager {
public:
	UserManager() = default;
	~UserManager() = default;

	void Init(const UINT32 maxUserCount_) {
		mMaxUserCnt = maxUserCount_;
		mUserObjPool = std::vector<User*>(mMaxUserCnt);

		for (auto i = 0; i < mMaxUserCnt; i++) {
			mUserObjPool[i] = new User();
			mUserObjPool[i]->Init(i);
		}
	}

	INT32 GetCurrentUserCnt() { return mCurrentUserCnt; }

	INT32 GetMaxUserCnt() { return mMaxUserCnt; }

	void IncreaseUserCnt() { mCurrentUserCnt++; }

	void DecreaseUserCnt() 
	{
		if (mCurrentUserCnt > 0) {
			mCurrentUserCnt--;
		}
	}

	// 유저 추가: User에 클라ID 부여하고 userid와 클라id 연결
	ERROR_CODE AddUser(char* userID_, int clientIndex_) {
		auto user_idx = clientIndex_;

		mUserObjPool[user_idx]->SetLogin(userID_);
		mUserIDDictionary.insert(std::pair<char*, int>(userID_, clientIndex_));

		return ERROR_CODE::NONE;

	}

	// userID로 User클래스 반환
	INT32 FindUserIndexByID(char* userID_) {

		if (auto res = mUserIDDictionary.find(userID_); res != mUserIDDictionary.end()) {
			return (*res).second;
		}

		return -1;
	}

	// 유저 삭제. Dictionary 삭제하고 User 정보 제거
	void DeleteUserInfo(User* user_) {
		mUserIDDictionary.erase(user_->GetUserId());
		user_->Clear();
	}

	// 클라이언트 인덱스로 User에 접그
	User* GetUserByConnIdx(INT32 clientIndex_) {
		return mUserObjPool[clientIndex_];
	}


private:
	UINT32 mMaxUserCnt = 0;
	UINT32 mCurrentUserCnt = 0;


	std::vector<User*> mUserObjPool;
	std::unordered_map<std::string, int> mUserIDDictionary;
};