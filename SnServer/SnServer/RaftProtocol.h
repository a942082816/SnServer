#pragma once


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
struct RequestVote
{
	int term;  //��ǰ���ں�
	int candidateId; // ��ѡ��id
	int lastLogIndex; //��ѡ�߱��ر���������־index
	int lastLogTerm;// ������־������
};

struct VoteResponse
{
	int term; //node�����ں� Candidate����ͶƱ�����node�����ں��������Լ������ں� 
	int voteGranted; //ͶƱ��� 1 for success
};



struct AppendEntries
{
	//Leader �� term
	int term; 
	// ����һ����־��ǰһ��index, ���������, Follower��index == prevLogIndex
	int prevLogIndex;
	//������־��ǰһ����־�����ں�term
	int prevLogTerm;
	//leader��ǰ�Ѿ�ȷ���ύ��״̬��FSM����־����index������ζ��FollowerҲ���԰�ȫ�ؽ�������index��ǰ����־�ύ
	int leaderCommit;
	//���������־��ϢЯ������־��������ʵ�������ֻ��һ��
	int entriesNum;
	LogEntry* entries;
};

struct AppendEntriesRespanse
{
	//��ǰ���ں�
	int term;
	//node�ɹ������־ʱ����ture����prev_log_index��prev_log_term���ȶԳɹ������򷵻�false
	int success;
	int currentIndex;
	int firstIndex;
};