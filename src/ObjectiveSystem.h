#pragma once

#include <queue>
#include <string>
#include <functional>
#include <unordered_set>
#include <fstream>

#include <Renderer.h>
#include <Font.h>

#include "Entities/Triggers.h"
#include "Utils/ObjectPool.h"

#include "PostBox.h"
#include "GameEvents.h"

class Objective
{

public:
    Objective(PostOffice &messenger)
        : m_post_office(&messenger) {}

    virtual void update(float dt) {};
    virtual void draw(Renderer &window, const TextureHolder &textures) = 0;
    void complete()
    {
        // m_post_office->send(ObjectiveFinishedEvent{m_id, ObjectiveEndCause::Completed});
        m_on_completion_callback();
    };
    void fail()
    {
        // m_post_office->send(ObjectiveFinishedEvent{m_id, ObjectiveEndCause::Failed});
        m_on_failure_callback();
    };
    virtual ~Objective() = default;
    bool isFinished() const;

public:
    PostOffice *m_post_office = nullptr;

    int m_id;
    float m_text_y_pos;
    std::function<void()> m_on_completion_callback = []() {};
    std::function<void()> m_on_failure_callback = []() {};

protected:
    Font *m_font = nullptr;
    bool m_is_finished = false;
};

class Quest;
class Task
{

public:
    Task(PostOffice &messenger, Quest *parent)
        : m_post_office(&messenger), m_parent(parent)
    {
    }

    virtual ~Task() = default;
    Task(const Task &other) = default;
    Task(Task &&other) = default;
    Task &operator=(const Task &other) = default;
    Task &operator=(Task &&other) = default;

    virtual void update(float dt) {};
    virtual void draw(Renderer &window, const TextureHolder &textures) = 0;

    void activate();
    void complete();
    void fail();

    bool isFinished() const;

public:

std::function<void()> m_on_completion_callback = []() {};
std::function<void()> m_on_failure_callback = []() {};

int m_id;
float m_text_y_pos = 100.f;

protected:
PostOffice *m_post_office = nullptr;
Quest *m_parent = nullptr;
Font *m_font = nullptr;

    bool m_is_finished = false;
    bool m_active = false;
};

class CompositeObjective : public Objective
{

public:
    CompositeObjective(PostOffice &messenger, std::vector<std::unique_ptr<Task>> &tasks) : Objective(messenger)
    {
        for (auto &task : tasks)
        {
            addTask(std::move(task));
        }
    }
    void onTaskCompletion(Task *completed_task)
    {
        m_tasks_count_to_complete--;
        if (m_tasks_count_to_complete == 0)
        {
            m_is_finished = true;
            complete();
        }
    }

    void addTask(std::unique_ptr<Task> sub_task)
    {
        m_tasks_count_to_complete++;

        sub_tasks.push_back(std::move(sub_task));
    }

    virtual void draw(Renderer &window, const TextureHolder &textures)
    {
        for (auto &task : sub_tasks)
        {
            task->draw(window, textures);
        }
    }

    int m_tasks_count_to_complete = 0;
    std::vector<std::unique_ptr<Task>> sub_tasks;
};

enum class QuestState
{
    NotStarted,
    Locked,
    Active,
    Completed,
    Failed
};

class Quest
{
public:
    float m_text_y_pos; //! TODO: will create some draw compotent some time

    Quest(PostOffice &messenger)
        : p_post_office(&messenger)
    {
    }

    void draw(Renderer& window, const TextureHolder& textures);

    void addTask(std::shared_ptr<Task> task, Task* precondition = nullptr)
    {
        assert(task);

        int task_internal_id = sub_tasks.size();
        task->m_id = task_internal_id;
        sub_tasks.emplace_back(task, precondition, std::vector<Task *>{});

        if (precondition)
        {
            sub_tasks.at(precondition->m_id).children.push_back(task.get());
        }
        else
        {
            //! if there is no precondition, the task must be active
            active_tasks_ids.insert(task_internal_id);
        }
    }

    void onTaskCompletion(Task *completed_task)
    {
        assert(active_tasks_ids.contains(completed_task->m_id));

        active_tasks_ids.erase(completed_task->m_id);

        for (auto child : sub_tasks.at(completed_task->m_id).children)
        {
            child->activate();
            active_tasks_ids.insert(child->m_id);
        }

        if (active_tasks_ids.empty())
        {
            completeQuest();
        }
    }

    void completeQuest()
    {
        m_state = QuestState::Completed;
        p_post_office->send(QuestCompletedEvent{m_id});
    }

    void start()
    {
        assert(sub_tasks.size() > 0);
        for (auto active_id : active_tasks_ids)
        {
            sub_tasks.at(active_id).task->activate();
        }
    }

private:
    bool checkCircularDependencies() const
    {
        return true;
    }

    struct TaskGraphNode
    {
        std::shared_ptr<Task> task;
        Task *precondition;
        std::vector<Task *> children;
    };

    PostOffice *p_post_office;

    int root_task_id = 0;
    std::vector<TaskGraphNode> sub_tasks;
    std::unordered_set<int> active_tasks_ids;

    int m_id = 0;

    QuestState m_state = QuestState::NotStarted;
    Task *primary_task;
};

class ReachSpotTask : public Task, public Observer<Trigger>
{

public:
    ReachSpotTask(GameObject &spot, Font &font, PostOffice &messenger, Quest *parent);
    virtual ~ReachSpotTask() = default;

    virtual void onObservation(Trigger *trig) override;
    // virtual void fail(Trigger* trig) override;
    virtual void draw(Renderer &window, const TextureHolder &textures) override;

private:
    utils::Vector2f m_location;
};

class SurveySpotTask : public Task, public Observer<Trigger>
{

public:
    SurveySpotTask(GameObject &spot, Font &font, PostOffice &messenger, Quest *parent);
    virtual ~SurveySpotTask() = default;

    virtual void onObservation(Trigger *trig) override;
    virtual void draw(Renderer &window, const TextureHolder &textures) override {}

private:
    float m_surveyed_timer = 0;
    GameObject &m_surveyed;
};

class DestroyEntityTask : public Task, public Observer<Trigger>
{

public:
    DestroyEntityTask(GameObject &target, Font &font, PostOffice &messenger, Quest *parent);
    virtual ~DestroyEntityTask() = default;

    virtual void onObservation(Trigger *trig) override;

    virtual void draw(Renderer &window, const TextureHolder &textures) override;

private:
    GameObject &m_target;
};

class DestroyNOfTypeTask : public Task
{

public:
    DestroyNOfTypeTask(ObjectType type, std::string name, int destroyed_target_count, Font &font, PostOffice &messenger, Quest *parent);
    virtual ~DestroyNOfTypeTask() = default;

    void entityDestroyed(ObjectType type, int id);

    virtual void draw(Renderer &window, const TextureHolder &textures) override;

private:
    ObjectType m_type;
    int m_destroyed_count = 0;
    int m_destroyed_target;

    std::unique_ptr<PostBox<EntityDiedEvent>> m_on_entity_destroyed;

    std::string m_entity_name;
};

class PostOffice;


class ObjectiveSystem
{

public:
    ObjectiveSystem(PostOffice &messanger);

    void add(std::shared_ptr<Quest> quest);
    void remove(int id);
    void draw(Renderer &window, const TextureHolder &textures);
    void update();
    bool allFinished() const;
    void entityDestroyed(ObjectType type, int id);

    void registerQuest(std::filesystem::path json);

private:
    // std::deque<std::shared_ptr<Objective>> m_objectives;
    utils::ObjectPool<std::shared_ptr<Objective>, 10> m_objectives;

    std::unordered_set<std::shared_ptr<Quest>> m_quests;

    std::unordered_set<int> active_quests_ids;

    bool m_all_quests_finished = false;

    PostOffice *p_messanger;

    std::unique_ptr<PostBox<EntityDiedEvent>> post_box;
    std::unique_ptr<PostBox<QuestCompletedEvent>> m_objectives_postbox;
};