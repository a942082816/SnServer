#pragma once
#include"stdafx.h"
#include"Reactor.h"
#include"TimeHandler.h"
#include"RaftProtocol.h"

struct LogEntry
{
	int index;
	int term;
	int type;
	int len;
	void* buffer;

	LogEntry& operator=(const LogEntry& right)
	{
		index = right.index;
		term = right.term;
		type = right.type;
		len = right.len;
		buffer = new char[len];
		memcpy(buffer, right.buffer, len);
	}
	~LogEntry()
	{
		if (buffer)
		{
			delete[] buffer;
		}
	}
	
};
struct NodeInfo
{
	void *udate;
	//����ÿһ������������Ҫ���͸�������һ����־��Ŀ������ֵ����ʼ��Ϊ�쵼���������ֵ��һ��
	int nextIndex;
	//����ÿһ�����������Ѿ����Ƹ�������־���������ֵ
	int matchIndex;
	//������ȡֵ�������Ĺ�ϵ 1:�û����и���ͶƱ 2:�û�����ͶƱȨ  3: �û��������µ���־
	int flags;
	//������Ӧ��idֵ�����ÿ̨������ȫ�ֶ���Ψһ��
	int id;
};

class RaftServer
{
public:
	enum class State{Follower, Candidate, Leader};

public:
	void periodic(int timeSinceLastPeriod)
	{
		//ѡ�ټ�ʱ��  follower  �յ����� ��0   Leader �������� ��0
		timeoutElapsed_ += timeSinceLastPeriod;
		if (state_ == State::Leader)
		{
			if (requestTimeout_ <= timeoutElapsed_)
			{
				//sendAppender todo
			}
		}
		else if (electionTimeout_ <= timeoutElapsed_)
		{
			if (nodes_.size() > 1)
			{
				//start election
			}
		}
		if (lastAppliedIndex_ < commitIndex_)
		{
			// commit raft log
		}
	}

	void becomeCandidate()
	{
		currentTerm_++;
		for (auto& node : nodes_)
		{
			//���Լ�ͶƱ
		}
		//���Լ�ͶƱ todo

		currentLeader_ = nullptr;

		state_ = State::Candidate;

		timeoutElapsed_ = std::rand() % electionTimeout_;

		for (auto& node : nodes_)
		{
			//send RequestVote
		}
	}

	void onRecvRequestVote(NodeInfo* node, const RequestVote& request)
	{
		if (currentTerm_ < request.term)
		{
			currentTerm_ = request.term;
			// become raft;
		}
		VoteResponse response;
		if (checkShouldGrantVote(request)) //��ҪͶƱ
		{
			voteFor_ = request.candidateId;
			response.voteGranted = 1;

			currentLeader_ = nullptr;
			timeoutElapsed_ = 0;
		}
		else
		{
			response.voteGranted = 0;
		}

		response.term = currentTerm_;
		//send response back
	}

	bool checkShouldGrantVote(const RequestVote& request)
	{
		if (request.term < currentTerm_)
		{
			return false;
		}
		if (voteFor_ != 0)
		{
			return false;
		}
		int currentIndex = logEntrys_[logEntrys_.size() - 1]->index;
		if (currentIndex == 0)
		{
			return true;
		}

		auto& logEntry = logEntrys_[logEntrys_.size() - 1];
		if (logEntry->term < request.lastLogTerm)
		{
			return true;
		}
		if (logEntry->term == request.lastLogTerm && currentIndex <= request.lastLogIndex)
		{
			return true;
		}
		return false;
	}

	bool onRecvRequestVoteResponse(NodeInfo* node, const VoteResponse& response)
	{
		if (state_ != State::Candidate)
		{
			return false;
		}
		//�յ��ķ���term���Լ���, �ָ�Follower
		if (response.term > currentTerm_)
		{
			currentTerm_ = response.term;
			state_ = State::Follower;
			return false;
		}
		//��ʱ��response
		else if (response.term < currentTerm_)
		{
			return 0;
		}
		//��ͶƱ��
		if (response.voteGranted == 1)
		{
			nodes_[node->id].flags |= 0x01;
			voteNum_++;
			if (voteNum_ > nodes_.size() / 2)
			{
				//become leader todo
				return true;
			}

		}
		return false;

	}

	bool sendAppendEntries(NodeInfo* node)
	{
		AppendEntries request;
		request.term = currentTerm_;
		request.leaderCommit = commitIndex_;
		request.prevLogIndex = 0;
		request.prevLogTerm = 0;
		request.entriesNum = 0;
		request.entries = NULL;

		int nextIndex = node->nextIndex;
		
		auto& logEntry = logEntrys_[nextIndex];
		
		request.entriesNum = 1;
		//copy to sendBuffer

		if (nextIndex > 1)
		{
			auto& prevLogEntry = logEntrys_[nextIndex - 1];
			request.prevLogIndex = nextIndex - 1;
			request.prevLogTerm = prevLogEntry.term;
		}

		//send to socket todo

	}

	bool onRecvAppendEntries(NodeInfo *node, const AppendEntries& request)
	{
		requestTimeout_ = 0;
		AppendEntriesRespanse response;
		response.term = currentTerm_;
		//����Լ���  ��ѡ�� �����յ���term���Լ���ȵ�����, ˵�����˳���Leader, ��ΪFollower
		if (state_ == State::Candidate && currentTerm_ == request.term)
		{
			voteFor_ = -1;
			state_ = State::Follower;
		}
		//term < leader
		if (currentTerm_ < request.term)
		{
			currentTerm_ = request.term;
			response.term = currentTerm_;
			state_ = State::Follower;
		}
		//˵�����ڵ�Leader ��ʱ��, �践��һ�����µ�term
		else if (request.term < currentTerm_)
		{
			goto FailWithCurrentIndex;
		}
		if (request.prevLogIndex > 0)
		{
			if (request.prevLogIndex > logEntrys_.size())
			{
				//˵����־�Ѿ����Leader, �践���Լ���ǰ��index
				goto FailWithCurrentIndex;
			}
			auto& spLogEntry = logEntrys_[request.prevLogIndex];

			if (spLogEntry.term != request.prevLogTerm)
			{
				logEntrys_.resize(request.prevLogIndex);
				response.currentIndex = request.prevLogIndex - 1;
				//
				goto Fail;
			}
		}
		/* ���ص���־��LeaderҪ�ࡣ�����ط�����������Leader���յ��˺ܶ�ͻ�������
		* ����û���ü�ͬ��ʱ����������������ʱ����������ɾ����Leader�����־ */
		if (request.entriesNum == 0 && request.prevLogIndex > 0 && request.prevLogIndex + 1 < logEntrys_.size())
		{
			logEntrys_.resize(request.prevLogIndex + 1);
		}

		response.currentIndex = request.prevLogIndex;
		int i = 0;
		/* �����������Ѿ��ڱ�����ӹ�����־*/
		for (; i < request.entriesNum; ++i)
		{
			 LogEntry* entry = &request.entries[i];
			 int indexEntry = request.prevLogIndex + 1 + i;
			 if (indexEntry > logEntrys_.size())
			 {
				 break;
			 }
			 if (logEntrys_[indexEntry].term != entry->term)
			 {
				 logEntrys_.resize(indexEntry);
				 break;
			 }
		}
		/* ������ȷ�ϵ�����־��ӵ����� */
		for (; i < request.entriesNum; ++i)
		{
			logEntrys_.push_back(request.entries[i]);
			response.currentIndex = request.prevLogIndex + 1 + i;
		}

		/* 4. ������Я����Leader�Ѿ��ύ��״̬������־����������ͬ��Ҳ�����������������
		*    ����Ϊ���������־������leader_commit�еĽ�С��*/
		if (commitIndex_ < request.leaderCommit)
		{
			int lastLogIndex = max(logEntrys_.size() -1, 1);
			commitIndex_ = min(lastLogIndex, request.leaderCommit);
		}

		currentLeader_ = node;
		response.success = 1;
		response.firstIndex = request.prevLogIndex + 1;
		//Send back
		return true;
	FailWithCurrentIndex:
		response.currentIndex = logEntrys_.size() - 1;
	Fail:
		response.success = 0;
		response.firstIndex = 0;
		return -1;
		//Sendback
	}


private:
	//���������һ��֪�������ں�
	int currentTerm_;
	//��¼�ڵ�ǰ�����ڸ��ĸ�CandidateͶ��Ʊ
	int voteFor_;

	std::vector<LogEntry> logEntrys_;


	int commitIndex_;
	//���Ӧ�õ�״̬������־��Ŀ����ֵ����ʼ��Ϊ 0������������
	int lastAppliedIndex_;

	State state_;
	//��ͶƱ������
	int voteNum_;
	//��ʱ��
	int timeoutElapsed_;
	//�����ڵ����Ϣ
	//std::vector<NodeInfo> nodes_;
	std::map<int, NodeInfo> nodes_;

	int electionTimeout_;
	int requestTimeout_;

	NodeInfo selfInfo_;
	NodeInfo* currentLeader_;
	/* ��raftʵ��ÿ��ֻ����һ�������������ø��ģ��ñ�����¼raft server
	* �Ƿ����ڽ������ø���*/
	int votingCfgChangeLogIndex_;



};