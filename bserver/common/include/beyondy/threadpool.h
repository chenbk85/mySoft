
class Job {
 public:
 protected:
  virtual void callback() = 0;
};

class ThreadPool {
 public:
  int submit(Job* job, long timeout);
  int submit(BatchJob* jobs, long timeout);
  int execute(Job* job, long timeout);
  int execute(BatchJob* jobs, long timeout);
};
