#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>
#include <Windows.h>
using namespace std;

struct EdgeInfo
{
	int from, to;
	int weight;
	EdgeInfo(int from = 0, int to = 0, int weight = 0) :
		from(from), to(to), weight(weight) {}
};

struct Result
{
	int roadIndex;
	int fare;
	Result(int roadIndex = 0, int fare = 0) :
		roadIndex(roadIndex), fare(fare) {}
};

// ���浥��Caseȫ������
struct GraphData
{
	EdgeInfo * roadInfo;
	int R;
	int N;
};

struct ResultItem
{
	bool hasSolution;
	int indexOfCase;
	int count;
	int maxFare;
	vector<Result> roadAndFare;
};

// ��¼ÿ���ڵ�����
int * countOfIndegree;

// ÿ���ڵ㵽���/�յ��������
int * maxDistanceToStart;
int * maxDistanceToEnd;

// ÿ���ڵ㵽���/�յ�ֱ��ж��������벻ͬ��·��
int * differenceFromStart;
int * differenceFromEnd;

// ��˳���¼�������Ľڵ�
vector<int> nodesByOrder;

// �洢ȫ���ߵ���Ϣ <x, y, c>
vector<EdgeInfo> edges;

// �洢�ڵ�����бߵ���Ϣ i:[<i, y1, c1>, <i, y2, c2>...]
vector<EdgeInfo> * edgesByNode;

// �洢ȫ�����
vector<ResultItem> resultsAll;

// �õ�����Case�����ݲ�������
void findAnswer(GraphData *g, int caseIndex)
{
	// һϵ�г�ʼ��
	ResultItem resultItem;
	resultItem.indexOfCase = caseIndex;

	for (int i = 0; i < g->N + 1; i++)
	{
		edgesByNode[i].clear();
		countOfIndegree[i] = 0;
	}

	edges.clear();

	// ͳ�Ƹ��ڵ������Ϣ�Լ����ڵ���Һ�̱���Ϣ
	for (int i = 0; i < g->R + 1; i++)
	{
		int x = g->roadInfo[i].from;
		int y = g->roadInfo[i].to;
		int c = g->roadInfo[i].weight;

		edgesByNode[x].push_back(EdgeInfo(x, y, c));
		edges.push_back(EdgeInfo(x, y, c));
		countOfIndegree[y] ++;
	}

	for (int i = 0; i < g->N + 1; i++)
	{
		differenceFromStart[i] = 0;
		differenceFromEnd[i] = 0;
		maxDistanceToStart[i] = 0;
		maxDistanceToEnd[i] = 0;
	}

	nodesByOrder.clear();
	queue<int> nodesToScan;

	// Ѱ�����Ϊ 0�Ľڵ����
	// Ҳ���Ǹò���Ŀ�����ҵ���ʼ�ڵ�
	for (int i = 1; i < g->N; i++)
	{
		if (countOfIndegree[i] == 0)
		{
			nodesToScan.push(i);
		}
	}
	// ����ʼ�ڵ㿪ʼ
	while (!nodesToScan.empty())
	{
		int nodeIndexTemp = nodesToScan.front();
		nodesToScan.pop();
		nodesByOrder.push_back(nodeIndexTemp);

		// ���ϸ��²���¼ÿ���ڵ㵽����������
		// ͬʱ�洢ÿ���ڵ㵽����ж��������벻ͬ��·��
		for (int i = 0; i < edgesByNode[nodeIndexTemp].size(); i++)
		{
			EdgeInfo edgeTemp = edgesByNode[nodeIndexTemp][i];
			if ((maxDistanceToStart[edgeTemp.to] != 0)
				&& (maxDistanceToStart[edgeTemp.to] !=
					(maxDistanceToStart[edgeTemp.from] + edgeTemp.weight)))
			{
				differenceFromStart[edgeTemp.to] += 1;
			}

			differenceFromStart[edgeTemp.to] += differenceFromStart[edgeTemp.from];

			maxDistanceToStart[edgeTemp.to] = max(maxDistanceToStart[edgeTemp.to]
				, maxDistanceToStart[edgeTemp.from] + edgeTemp.weight);

			countOfIndegree[edgeTemp.to] -= 1;
			// �����Ϊ0����ӣ�������һ��ѭ��
			if (countOfIndegree[edgeTemp.to] == 0)
			{
				nodesToScan.push(edgeTemp.to);
			}
		}
	}

	// ����ÿ���ڵ㵽�յ����Զ����
	// ͬʱ��¼ÿ���ڵ㵽�յ���벻ͬ��·������
	for (int i = nodesByOrder.size() - 1; i >= 0; i--)
	{
		int nodeTemp = nodesByOrder[i];
		if (nodeTemp == g->N)
		{
			// �յ�ڵ��Թ�
			continue;
		}
		for (EdgeInfo edgeTemp : edgesByNode[nodeTemp])
		{
			maxDistanceToEnd[edgeTemp.from] = max(maxDistanceToEnd[edgeTemp.from]
				, maxDistanceToEnd[edgeTemp.to] + edgeTemp.weight);
			differenceFromEnd[edgeTemp.from] += differenceFromEnd[edgeTemp.to];
		}
		if (edgesByNode[nodeTemp].size() != 0)
		{
			EdgeInfo edgeTemp = edgesByNode[nodeTemp][0];
			int distanceTemp = maxDistanceToEnd[edgeTemp.to] + edgeTemp.weight;

			// ���沿���ǵõ��ýڵ��Ҳ��ж��ٸ�·�߾��벻һ��
			for (int j = 0; j < edgesByNode[nodeTemp].size(); j++)
			{
				if (maxDistanceToEnd[edgesByNode[nodeTemp][j].to] + edgesByNode[nodeTemp][j].weight != distanceTemp)
				{
					differenceFromEnd[nodeTemp] += 1;
				}
			}
		}
	}

	// �������Ѿ�ȷ����ÿ���ڵ������ж��ٲ���ͬ��·���Լ����Ҳ�
	for (int i = 1; i <= g->N; i++)
	{
		// ������������޽�ģ������Ļ��ڵ�i [1~N]��Ҫͬʱ�޸��������Ҳ࣬����������
		if (differenceFromStart[i] > 0 && differenceFromEnd[i] > 0)
		{
			resultItem.hasSolution = false;
			resultsAll.push_back(resultItem);
			return;
		}
	}

	vector<Result> results;

	for (int i = 0; i < g->R; i++)
	{
		EdgeInfo edgeTemp = edges[i];

		// ���ĳ���ߵ���㵽ͼ�����û�о��벻ͬ��·�����иñߵ��յ㵽ͼ���յ�Ҳû�о��벻ͬ��·��
		// ��ñߵ������յ�֮����ܴ�����Ҫ�޸ĵı�
		if (differenceFromStart[edgeTemp.from] == 0 && differenceFromEnd[edgeTemp.to] == 0 &&
			(differenceFromStart[edgeTemp.to] > 0 || differenceFromEnd[edgeTemp.from] > 0))
		{
			// ��Ҫ���ӵ��շѵ��� H - x��start���ֵ - y��end���ֵ - <x, y>�����Ȩ��ֵ
			// �������Ϊ�����Ȩ�ؼ��϶��ٵ��շѲ����ڲ��޸������ҵ�����´ﵽ��ֵ H
			int fare = maxDistanceToStart[g->N] -
				(maxDistanceToStart[edgeTemp.from] + maxDistanceToEnd[edgeTemp.to] + edgeTemp.weight);
			if (fare > 0)
			{
				// ������
				results.push_back(Result(i + 1, fare));
			}
		}
	}

	resultItem.hasSolution = true;
	resultItem.indexOfCase = caseIndex;
	resultItem.count = results.size();
	resultItem.maxFare = maxDistanceToStart[g->N];
	for (int i = 0; i < results.size(); i++)
	{
		resultItem.roadAndFare = results;
	}
	resultsAll.push_back(resultItem);
}

// ʹ�ö�̬�����ʼ��һЩ����
void initVariables(int N, int R)
{
	// ÿ���ڵ�����
	delete[] countOfIndegree;
	countOfIndegree = NULL;
	countOfIndegree = new int[N + 1];

	// ÿ���ڵ㵽����������
	delete[] maxDistanceToStart;
	maxDistanceToStart = NULL;
	maxDistanceToStart = new int[N + 1];

	// ÿ���ڵ㵽�յ��������
	delete[] maxDistanceToEnd;
	maxDistanceToEnd = NULL;
	maxDistanceToEnd = new int[N + 1];

	// ÿ���ڵ㵽��㲻ͬ�����·������
	delete[] differenceFromStart;
	differenceFromStart = NULL;
	differenceFromStart = new int[N + 1];

	// ÿ���ڵ㵽�յ㲻ͬ�����·������
	delete[] differenceFromEnd;
	differenceFromEnd = NULL;
	differenceFromEnd = new int[N + 1];

	// ���սڵ�ֵ������ȫ����̱�
	edgesByNode->clear();
	delete[] edgesByNode;
	edgesByNode = NULL;
	edgesByNode = new vector<EdgeInfo>[N + 1];
}

// ��ӡ��ǰͼȫ������
void printCaseData(GraphData *g)
{
	cout << g->N << " " << g->R << endl;
	for (int i = 0; i < g->R; i++)
	{
		cout << g->roadInfo[i].from << " " << g->roadInfo[i].to << " " << g->roadInfo[i].weight << endl;
	}
}

// �ͷ��ڴ�
void clearMemory(GraphData *g)
{
	// �ͷŸ�case�ڴ�
	delete[] countOfIndegree;
	countOfIndegree = NULL;
	delete[] maxDistanceToEnd;
	maxDistanceToEnd = NULL;
	delete[] maxDistanceToStart;
	maxDistanceToStart = NULL;
	delete[] differenceFromEnd;
	differenceFromEnd = NULL;
	delete[] differenceFromStart;
	differenceFromStart = NULL;
	delete[] edgesByNode;
	edgesByNode = NULL;
	delete[] g->roadInfo;
	g->roadInfo = NULL;
}

// ���ļ��ж�ȡ������
void readFileAndEval(string filePath)
{
	int indexStart = 0;
	int indexEnd = 0;
	int indexNow = 0;
	int indexInCase = 0;
	int caseIndex = 0;

	// ������ʼֵ�����ⱨ��
	edgesByNode = new vector<EdgeInfo>[1];
	countOfIndegree = new int[1];
	maxDistanceToStart = new int[1];
	maxDistanceToEnd = new int[1];
	differenceFromStart = new int[1];
	differenceFromEnd = new int[1];

	GraphData graphData;
	graphData.roadInfo = new EdgeInfo[1];

	ifstream inFile(filePath);
	if (!inFile.is_open())
	{
		cout << "fail to read this file: " << filePath << endl;
		clearMemory(&graphData);
		return;
	}

	while (true)
	{
		if (indexNow == indexStart)
		{
			// ��ȡ����Case��N��Rֵ
			int N, R;
			inFile >> N;
			if (N == 0)
			{
				clearMemory(&graphData);
				break;
			}
			inFile >> R;

			graphData.N = N;
			graphData.R = R;

			delete[] graphData.roadInfo;
			graphData.roadInfo = NULL;
			graphData.roadInfo = new EdgeInfo[R + 1];

			initVariables(N, R);

			// ���㱾��Case��ȡ�յ��±�
			indexNow += 2;
			indexEnd = indexStart + 2 + R * 3;
		}
		if (indexNow <= indexEnd)
		{
			int x, y, c;
			inFile >> x;
			inFile >> y;
			inFile >> c;

			graphData.roadInfo[indexInCase].from = x;
			graphData.roadInfo[indexInCase].to = y;
			graphData.roadInfo[indexInCase].weight = c;
			indexInCase++;
			indexNow += 3;

			if (indexNow == indexEnd)
			{
				indexStart = indexEnd;
				indexInCase = 0;
				// ��ȡ���ݿ��ӻ�
				// printCaseData(&graphData);
				findAnswer(&graphData, ++caseIndex);
			}
		}
	}
	inFile.close();
}

// ��ӡȫ��������
void printAllResults()
{
	for (int i = 0; i < resultsAll.size(); i++)
	{
		if (resultsAll[i].hasSolution == false)
		{
			cout << "Case " << resultsAll[i].indexOfCase << ": No solution" << endl;
			continue;
		}
		cout << "Case " << resultsAll[i].indexOfCase << ": " << resultsAll[i].count << " " << resultsAll[i].maxFare << endl;
		for (int j = 0; j < resultsAll[i].roadAndFare.size(); j++)
		{
			cout << resultsAll[i].roadAndFare[j].roadIndex << " " << resultsAll[i].roadAndFare[j].fare << endl;
		}
	}
}

int main()
{
	string pathReal = "data/fare.txt";
	readFileAndEval(pathReal);
	printAllResults();
	return 0;
}