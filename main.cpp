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
    
    // unoffical api
    float value;

    bool operator> (const SampleData& other) const
    {
        return value > other.value;
    }

    bool isDiagnosed()
    {
        return health > -1;
    }

    int getTotalCost()
    {
        if (isDiagnosed())
        {
            auto totalCost = 0;
            for (const auto& cost : costs)
            {
                totalCost += cost;
            }
            return totalCost;
        }
        else
        {
            return -1;
        }
    }

    float getValue()
    {
        if (isDiagnosed())
        {
            return static_cast<float>(health) / static_cast<float>(getTotalCost());
        }
        else
        {
            return 0.0f;
        }
    }

    void print()
    {
        cerr << "============ Sample " << sampleId << " ============" << endl;
        cerr << "Carried by: " << carriedBy << endl;
        cerr << "Rank: " << rank << endl;
        cerr << "Health: " << health << endl;
        cerr << "Diagnosed:" << isDiagnosed() << endl;
        cerr << "Value: " << getValue() << endl;
        cerr << "Costs: ";
        for (const auto& cost : costs)
        {
            cerr << cost << " ";
        }
        cerr << endl;
    }
};

struct PlayerData
{
    string target;
    int score;
    vector<int> storages;
};

class State
{
public:
    State()
    : NAME("INITIAL")
    {
        cerr << "Starting at " << NAME << endl;
    }

    State(std::string name)
    : NAME(name)
    {
        cout << "GOTO " << NAME << endl;
    }

    virtual ~State()
    {
        cerr << "Leaving " << NAME  << endl;
    }
    virtual unique_ptr<State> next() = 0;
    virtual bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) = 0;
    const std::string NAME;
};

class InitialState : public State
{
public:
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override
    {
        return false;
    };
    unique_ptr<State> next() override;
};

class SampleState : public State
{
public:
    SampleState()
    : State("SAMPLES"){}
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<State> next() override;
};


class DiagnosisState : public State
{
public:
    DiagnosisState()
    : State("DIAGNOSIS"){}
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<State> next() override;
};

class MoleculesState : public State
{
public:
    MoleculesState()
    : State("MOLECULES"){}
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<State> next() override;
};

class LaboratoryState : public State
{
public:
    LaboratoryState()
    : State("LABORATORY"){}
    bool work(const vector<PlayerData> players = vector<PlayerData>(), const vector<SampleData> samples = vector<SampleData>()) override;
    unique_ptr<State> next() override;
    bool isComplete(const vector<int>& sampleCosts, const vector<int>& molecules);
};

// =====================
// InitialState
// =====================
unique_ptr<State> InitialState::next()
{
    return make_unique<SampleState>();
}

// =====================
// SampleState
// =====================
bool SampleState::work(const vector<PlayerData> players, const vector<SampleData> samples)
{
    /*
    Rank 1; min value: 0.2,  max value: 3.34
    Rank 2; min value: 2,    max value: 6
    Rank 3; min value: 2.14, max value: 7.14
    */
    if (3 > count_if(samples.begin(), samples.end(), [](auto sample) { return sample.carriedBy == 0;}))
    {
        cout << "CONNECT " << 2 << endl;
        return true;
    }

    return false;
}
unique_ptr<State> SampleState::next()
{
    return make_unique<DiagnosisState>();
}

// =====================
// DiagnosisState
// =====================
bool DiagnosisState::work(const vector<PlayerData> players, const vector<SampleData> samples)
{
    vector<SampleData> data = samples;
    std::sort(data.begin(), data.end(), greater<SampleData>());

    auto undiagnosedSampleIter = find_if(samples.begin(), samples.end(), [](auto sample)
    {
        return sample.carriedBy == 0 && !sample.isDiagnosed();
    });

    if (undiagnosedSampleIter != samples.end())
    {
        cout << "CONNECT " << undiagnosedSampleIter->sampleId << endl;
        return true;
    }
    else
    {
        return false;
    }
}
unique_ptr<State> DiagnosisState::next()
{
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
unique_ptr<State> MoleculesState::next()
{
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
unique_ptr<State> LaboratoryState::next()
{
    return make_unique<DiagnosisState>();
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

    unique_ptr<State> m_state;
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

        cerr << "Number of samples in game: " << sampleCount << endl;
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
            // cerr << "ID: " << sampleId << " Points: " << health << " Points per resource: " << sample.value << endl;
            sample.print();

            samples.push_back(sample);
        }
        //cerr << "Input parsing time: " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() << endl;

        stateMachine.advance(players, samples);
        //cerr << "Total time: " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() << endl;
    }
}
