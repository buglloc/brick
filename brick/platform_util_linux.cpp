#include <string>
#include <unistd.h>
#include <signal.h>

#include <include/wrapper/cef_helpers.h>
#include <sys/stat.h>

#include "helper.h"

namespace {

    // Set the calling thread's signal mask to new_sigmask and return
    // the previous signal mask.
    sigset_t
    SetSignalMask(const sigset_t& new_sigmask) {
      sigset_t old_sigmask;
      pthread_sigmask(SIG_SETMASK, &new_sigmask, &old_sigmask);
      return old_sigmask;
    }

    bool
    LaunchProcess(const std::vector<std::string>& args) {
      sigset_t full_sigset;
      sigfillset(&full_sigset);
      const sigset_t orig_sigmask = SetSignalMask(full_sigset);

      pid_t pid;
      pid = fork();

      // Always restore the original signal mask in the parent.
      if (pid != 0) {
        SetSignalMask(orig_sigmask);
      }

      if (pid < 0) {
        LOG(ERROR)
          << "LaunchProcess: failed fork";
        return false;
      } else if (pid == 0) {
        // Child process

        // DANGER: fork() rule: in the child, if you don't end up doing exec*(),
        // you call _exit() instead of exit(). This is because _exit() does not
        // call any previously-registered (in the parent) exit handlers, which
        // might do things like block waiting for threads that don't even exist
        // in the child.

        // ToDo: Put environment like Google chrome
        //      options.allow_new_privs = true;
//      // xdg-open can fall back on mailcap which eventually might plumb through
//      // to a command that needs a terminal.  Set the environment variable telling
//      // it that we definitely don't have a terminal available and that it should
//      // bring up a new terminal if necessary.  See "man mailcap".
//      options.environ["MM_NOTTTY"] = "1";
//
//      // We do not let GNOME's bug-buddy intercept our crashes.
//      char* disable_gnome_bug_buddy = getenv("GNOME_DISABLE_CRASH_DIALOG");
//      if (disable_gnome_bug_buddy &&
//         disable_gnome_bug_buddy == std::string("SET_BY_GOOGLE_CHROME"))
//        options.environ["GNOME_DISABLE_CRASH_DIALOG"] = std::string();

        std::vector<char*> argv(args.size() + 1, NULL);
        for (size_t i = 0; i < args.size(); ++i) {
          argv[i] = const_cast<char*>(args[i].c_str());
        }

        execvp(argv[0], &argv[0]);

        LOG(ERROR)
           << "LaunchProcess: failed to execvp:" << argv[0];
        _exit(127);
      }

      return true;
    }

    void
    XDGUtil(const std::string& util, const std::string& arg) {
      std::vector<std::string> argv;
      argv.push_back(util.c_str());
      argv.push_back(arg.c_str());
      LaunchProcess(argv);
    }

    void
    XDGOpen(const std::string& path) {
      XDGUtil("xdg-open", path);
    }

    void
    XDGEmail(const std::string& email) {
      XDGUtil("xdg-email", email);
    }

}  // namespace

namespace platform_util {

    void
    OpenExternal(std::string url) {
      if (url.find("mailto:") == 0)
        XDGEmail(url);
      else
        XDGOpen(url);
    }

    bool
    IsPathExists(std::string path) {
      struct stat stat_data;
      return stat(path.c_str(), &stat_data) != -1;
    }

    bool
    MakeDirectory(std::string path) {
      if (IsPathExists(path))
        return true;

      size_t pre=0, pos;
      std::string dir;

      if (path[path.size()-1] != '/'){
        // force trailing / so we can handle everything in loop
        path += '/';
      }

      while ((pos = path.find_first_of('/', pre)) !=std::string::npos){
        dir = path.substr(0, pos++);
        pre = pos;

        if (dir.size()==0)
          continue;

        if (mkdir(dir.c_str(), 0700) && errno != EEXIST) {
          return false;
        }
      }

      return true;
    }

}  // namespace platform_util
