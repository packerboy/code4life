#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>
#include <chrono>
#include <numeric>

using namespace std;

/**
 * Bring data on patient samples from the diagnosis machine to the laboratory with enough molecules to produce medicine!
 **/

struct SampleData
{
    int sampleId;
    int carriedBy;
    int rank;
    string expertiseGain;
    int health;
    vector<int> costs;
    float value;

    bool operator> (const SampleData& other) const
    {
        return value > other.value;
    }
};

struct PlayerData
{
    string target;
    int score;
    vector<int> storages;
};

class IState
{
public:

    virtual unique_ptr<IState> next() = 0;
    virtual bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) = 0;
    virtual ~IState(){};
};

class InitialState : public IState
{
public:
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override
    {
        return false;
    };
    unique_ptr<IState> next() override;
};

class DiagnosisState : public IState
{
public:
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<IState> next() override;
};

class MoleculesState : public IState
{
public:
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<IState> next() override;
};

class LaboratoryState : public IState
{
public:
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<IState> next() override;
    bool isComplete(const vector<int>& sampleCosts, const vector<int>& molecules);
};

// =====================
// InitialState
// =====================
unique_ptr<IState> InitialState::next()
{
    cout << "GOTO DIAGNOSIS" << endl;
    return make_unique<DiagnosisState>();
}

// =====================
// DiagnosisState
// =====================
bool DiagnosisState::work(const vector<PlayerData> players, const vector<SampleData> samples)
{
    vector<SampleData> data = samples;
    std::sort(data.begin(), data.end(), greater<SampleData>());

    auto carriedSamples = count_if(data.begin(), data.end(), [](auto datum){ return datum.carriedBy == 0;});
    if (carriedSamples >= 3)
    {
        return false;
    }
    for (auto datum : data)
    {
        if (-1 == datum.carriedBy)
        {
            cerr << "Download sample with id: " << datum.sampleId << " value: " << datum.value << endl;
            cout << "CONNECT " << datum.sampleId << endl;
            return true;
        }
    }
    return false;
}
unique_ptr<IState> DiagnosisState::next()
{
    cout << "GOTO MOLECULES" << endl;
    return make_unique<MoleculesState>();
}

// =====================
// MoleculesState
// =====================
bool MoleculesState::work(const vector<PlayerData> players, const vector<SampleData> samples)
{
    const PlayerData& myPlayerData = players.at(0);
    // we are done if we are carrying 10 molecules.
    if (accumulate(myPlayerData.storages.begin(), myPlayerData.storages.end(), 0) >= 10)
    {
        return false;
    }

    vector<SampleData> data = samples;
    std::sort(data.begin(), data.end(), greater<SampleData>());

    auto it = find_if(data.begin(), data.end(), [](auto sample){ return 0 == sample.carriedBy; });

    do
    {
        if (it != data.end())
        {
            auto size = it->costs.size();
            assert(size == myPlayerData.storages.size());
            for (auto i = 0; i < size; i++)
            {
                if (it->costs.at(i) > myPlayerData.storages.at(i))
                {
                    char types[] = "ABCDE";
                    cout << "CONNECT " << types[i] << endl;
                    return true;
                }
            }
        }
    }
    while (++it != data.end());

    return false;
}
unique_ptr<IState> MoleculesState::next()
{
    cout << "GOTO LABORATORY" << endl;
    return make_unique<LaboratoryState>();
}

// =====================
// LaboratoryState
// =====================
bool LaboratoryState::work(const vector<PlayerData> players, const vector<SampleData> samples)
{
    vector<SampleData> data = samples;
    std::sort(data.begin(), data.end(), greater<SampleData>());

    auto it = find_if(data.begin(), data.end(), [](auto sample){ return 0 == sample.carriedBy; });
    if (it != data.end())
    {
        if (isComplete(it->costs, players.at(0).storages))
        {
            cout << "CONNECT " << it->sampleId << endl;
            return true;
        }
    }
    return false;
}
unique_ptr<IState> LaboratoryState::next()
{
    cout << "GOTO DIAGNOSIS" << endl;
    return unique_ptr<IState>(new DiagnosisState());
}
bool LaboratoryState::isComplete(const vector<int>& sampleCosts, const vector<int>& molecules)
{
    auto size = sampleCosts.size();
    assert(molecules.size() == size);
    for (auto i = 0; i < size; i++)
    {
        if (sampleCosts.at(i) > molecules.at(i))
        {
            return false;
        }
    }
    return true;
}

class StateMachine
{
public:
    StateMachine()
    : m_state(make_unique<InitialState>())
    {
    }

    void advance(const vector<PlayerData>& players, const vector<SampleData>& samples)
    {
        if (!m_state->work(players, samples))
        {
            m_state = move(m_state->next());
        }
    }

    unique_ptr<IState> m_state;
};

int main()
{
    int projectCount;
    cin >> projectCount;
    cin.ignore();
    for (int i = 0; i < projectCount; i++)
    {
        int a;
        int b;
        int c;
        int d;
        int e;
        cin >> a >> b >> c >> d >> e;
        cin.ignore();
    }

    StateMachine stateMachine;

    // game loop
    while (1)
    {
        const chrono::time_point<std::chrono::steady_clock> start = chrono::steady_clock::now();
        vector<PlayerData> players;
        for (int i = 0; i < 2; i++)
        {
            string target;
            int eta;
            int score;
            int storageA;
            int storageB;
            int storageC;
            int storageD;
            int storageE;
            int expertiseA;
            int expertiseB;
            int expertiseC;
            int expertiseD;
            int expertiseE;
            cin >> target >> eta >> score >> storageA >> storageB >> storageC >> storageD >> storageE >> expertiseA >> expertiseB >> expertiseC >> expertiseD >> expertiseE;
            cin.ignore();

            PlayerData player;
            player.target = target;
            player.score = score;
            player.storages = {storageA, storageB, storageC, storageD, storageE};

            players.push_back(player);
        }

        int availableA;
        int availableB;
        int availableC;
        int availableD;
        int availableE;
        cin >> availableA >> availableB >> availableC >> availableD >> availableE;
        cin.ignore();

        int sampleCount;
        cin >> sampleCount;
        cin.ignore();

        cerr << "Number of samples: " << sampleCount << endl;
        vector<SampleData> samples;
        for (int i = 0; i < sampleCount; i++)
        {
            int sampleId;
            int carriedBy;
            int rank;
            string expertiseGain;
            int health;
            int costA;
            int costB;
            int costC;
            int costD;
            int costE;
            cin >> sampleId >> carriedBy >> rank >> expertiseGain >> health >> costA >> costB >> costC >> costD >> costE;
            cin.ignore();

            SampleData sample;
            sample.sampleId = sampleId;
            sample.carriedBy = carriedBy;
            sample.rank = rank;
            sample.expertiseGain = expertiseGain;
            sample.health = health;
            sample.costs.push_back(costA);
            sample.costs.push_back(costB);
            sample.costs.push_back(costC);
            sample.costs.push_back(costD);
            sample.costs.push_back(costE);
            sample.value = static_cast<float>(health) / static_cast<float>((costA + costB + costC + costD + costE));
            // cerr << "ID: " << sampleId << " Points: " << health << " Points per resource: " << sample.value << endl;

            samples.push_back(sample);
        }
        cerr << "Input parsing time: " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() << endl;

        stateMachine.advance(players, samples);
        cerr << "Total time: " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() << endl;
    }
}
