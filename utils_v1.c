#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>

#include "utils_v1.h"

#define BUF_LEN 256

//******************************************************//
// LECTURE CLAVIER
//******************************************************//

// Version modifiée de la fonction: litEtValideChaine (cf. TP6 de l'UE I2011)

int readLimitedLine(char *s, int sz)
{
  while (fgets(s, sz, stdin) != NULL)
  {
    // Traitement d'une ligne trop longue
    if (s[strlen(s) - 1] != '\n')
    {
      printf("Ligne trop longue (max %d caracteres). Recommencez.\n", sz - 2);
      // vidage du buffer de lecture bloc par bloc
      while (fgets(s, sz, stdin) && s[strlen(s) - 1] != '\n')
        ;
      // OU vidage du buffer de lecture caractere par caractere:
      // while (getchar() != '\n');
      continue;
    }

    // Chaine valide: suppression de '\n' et renvoi du nombre de caractères de la chaine
    s[strlen(s) - 1] = '\0';
    return strlen(s);
  }

  // Cas d'erreur ou de fin de fichier
  return -1;
}

// Anciennement: lireLigne (cf. TP1)
// Pour une vidéo sur cette fonction: http://youtu.be/Zuyhvnwmlao?hd=1

char *readLine()
{
  char buf[BUF_LEN];
  char *s = NULL;
  bool fin = false;

  while (fgets(buf, BUF_LEN, stdin))
  {
    // Traitement d'un bloc de caractères lus sur stdin
    int taille = strlen(buf);
    if (buf[taille - 1] == '\n')
    {
      taille--;
      buf[taille] = '\0';
      fin = true;
    }

    if (s == NULL)
    {
      // Première allocation de la chaîne s
      s = (char *)malloc((taille + 1) * sizeof(char));
      if (s == NULL)
        return NULL;
      // Copie des caractères du buffer dans s
      strcpy(s, buf);
    }
    else
    {
      // Réallocation de la chaîne s
      s = (char *)realloc(s, (strlen(s) + taille + 1) * sizeof(char));
      if (s == NULL)
        return NULL;
      // Concaténation des caractères du buffer à la fin de s
      strcat(s, buf);
    }

    if (fin)
      // Fin de la ligne lue sur stdin
      return s;
  }

  // Cas d'erreur ou de fin de fichier
  return NULL;
}

//******************************************************
// AFFICHAGE DE MESSAGES EN COULEUR
//******************************************************

void printOk(char *s)
{
  colorOn(1, GREEN_TEXT);
  fprintf(stdout, "%s", s);
  colorOff();
}

void printError(char *s)
{
  colorOn(1, RED_TEXT);
  fprintf(stdout, "%s", s);
  colorOff();
}

void printColor(char *format, char *s, int color)
{
  colorOn(1, color);
  fprintf(stdout, format, s);
  colorOff();
}

//******************************************************
// DATE AND TIME
//******************************************************

char *getTime()
{
  time_t t;
  time_t ret = time(&t);
  checkNeg(ret, "ERROR getTime");

  char *s = ctime(&t);
  checkCond(s == NULL, "ERROR getTime");

  size_t sz = strlen(s);
  s[sz - 1] = '\0';

  return s;
}

//******************************************************
// RANDOM INTEGER
//******************************************************

static unsigned generateRandomSeed()
{
  unsigned seed;
  FILE *fd = fopen("/dev/urandom", "r"); // entropy file in Unix systems
  checkCond(fd == NULL, "ERROR fopen");

  size_t ret1 = fread(&seed, sizeof(unsigned), 1, fd);
  checkCond(ret1 == 0, "ERROR fread");

  int ret2 = fclose(fd);
  checkCond(ret2 == EOF, "ERROR fclose");
  return seed;
}

int randomIntBetween(int valMin, int valMax)
{
  // Initialization of 'random seed'
  srand(generateRandomSeed()); // srand defined in <stdlib.h>

  // Generation of random value
  int nombre = valMin + (int)(rand() / (RAND_MAX + 1.0) * (valMax - valMin + 1)); // rand and RAND_MAX defined in <stdlib.h>
  return nombre;
}

//*****************************************************************************
// MALLOC
//*****************************************************************************

void *smalloc(size_t size)
{
  void *res = malloc(size);
  checkCond(size != 0 && res == NULL, "ERROR MALLOC");
  return res;
}

//*****************************************************************************
// CHECK
//*****************************************************************************

void checkCond(bool cond, char *msg)
{
  if (cond)
  {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

void checkNeg(int res, char *msg)
{
  checkCond(res < 0, msg);
}

void checkNull(void *res, char *msg)
{
  checkCond(res == NULL, msg);
}

//*****************************************************************************
// IO
//*****************************************************************************

int sopen(const char *pathname, int flags, mode_t mode)
{
  int resv = open(pathname, flags, mode);
  checkNeg(resv, "Error OPEN");
  return resv;
}

int sclose(int fd)
{
  int resv = close(fd);
  checkNeg(resv, "Error CLOSE");
  return resv;
}

ssize_t sread(int fd, void *buf, size_t count)
{
  ssize_t r = read(fd, buf, count);
  checkNeg(r, "Error READ");
  return r;
}

ssize_t swrite(int fd, const void *buf, size_t count)
{
  ssize_t r = write(fd, buf, count);
  checkNeg(r, "Error WRITE");
  return r;
}

void nwrite(int fd, const void *buf, size_t count)
{
  char *cbuf = (char *)buf;
  int s = swrite(fd, cbuf, count);
  int i = s;
  while (s != 0 && i != count)
  {
    s = swrite(fd, cbuf + i, count - i);
    i += s;
  }

  if (i != count)
  {
    fprintf(stderr, "Unable to write %lu byte(s)\n", count);
    exit(EXIT_FAILURE);
  }
}
/* TO DELETE 29/03/2024
int readLimitedLineOnFile(int fd, char* s, int sz) {
  FILE *file = fdopen(fd, "r");
  char *ret = fgets(s, sz, file);
  if (ret == NULL) {
    return 0;
  }

  // Traitement d'une ligne vide
  if (strlen(s) == 1) {
    return 0;
  }

  // Traitement d'une ligne trop longue
  if (s[strlen(s)-1] != '\n') {
    return 0;
  }

  // Chaine valide: suppression de '\n' et renvoi du nombre de caractères de la chaine
  s[strlen(s)-1] = '\0';
  return strlen(s);
}
*/

char **readFileToTable(int fd)
{

  char buffer[1024];
  ssize_t bytes_read;
  ssize_t line_start = 0;
  size_t num_lines = 0;
  char **lines = NULL;

  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
  {
    for (ssize_t i = 0; i < bytes_read; ++i)
    {
      if (buffer[i] == '\n')
      {
        // Allocate memory for the line
        char *line = (char *)malloc(i - line_start + 1);
        if (!line)
        {
          perror("Error allocating memory");
          return NULL;
        }
        // Copy the line into the new buffer
        memcpy(line, buffer + line_start, i - line_start);
        line[i - line_start] = '\0'; // Add the null terminator

        // Resize the array of lines
        num_lines++;
        lines = (char **)realloc(lines, num_lines * sizeof(char *));
        if (!lines)
        {
          perror("Error reallocating memory");
          return NULL;
        }
        lines[num_lines - 1] = line;

        line_start = i + 1;
      }
    }
  }

  if (bytes_read == -1)
  {
    perror("Error reading the file");
    return NULL;
  }
  return lines;
}

//*****************************************************************************
// FORK
//*****************************************************************************

pid_t sfork()
{
  int childId = fork();
  checkCond(childId == -1, "Fork failed");
  return childId;
}

pid_t fork_and_run0(void (*run)(void))
{
  int childId = fork();
  checkNeg(childId, "Fork failded");

  if (childId == 0)
  {
    (*run)();
    exit(EXIT_SUCCESS);
  }

  return childId;
}

pid_t fork_and_run1(void (*run)(void *), void *arg0)
{
  int childId = fork();
  checkNeg(childId, "Fork failded");

  if (childId == 0)
  {
    (*run)(arg0);
    exit(EXIT_SUCCESS);
  }

  return childId;
}

pid_t fork_and_run2(void (*run)(void *, void *), void *arg0, void *arg1)
{
  int childId = fork();
  checkNeg(childId, "Fork failded");

  if (childId == 0)
  {
    (*run)(arg0, arg1);
    exit(EXIT_SUCCESS);
  }

  return childId;
}

pid_t fork_and_run3(void (*run)(void *, void *, void *), void *arg0, void *arg1, void *arg2)
{
  int childId = fork();
  checkNeg(childId, "Fork failded");

  if (childId == 0)
  {
    (*run)(arg0, arg1, arg2);
    exit(EXIT_SUCCESS);
  }

  return childId;
}

pid_t swaitpid(pid_t pid, int *wstatus, int option)
{
  pid_t waitId = waitpid(pid, wstatus, option);
  checkNeg(waitId, "Error WAITPID");
  return waitId;
}

pid_t swait(int *wstatus)
{
  pid_t waitId = wait(wstatus);
  checkNeg(waitId, "Error WAIT");
  return waitId;
}

int sexecl(const char *path, const char *arg, ...)
{
  va_list(valist);

  // count the number of args
  va_start(valist, arg);
  int cpt = 0;
  char *tmp = va_arg(valist, char *);
  while (tmp != NULL)
  {
    cpt++;
    tmp = va_arg(valist, char *);
  }
  va_end(valist);

  // copy the args
  char **argv = smalloc((cpt + 2) * sizeof(char *));
  va_start(valist, arg);
  argv[0] = (char *)arg;
  for (int i = 1; i != cpt + 1; i++)
  {
    tmp = va_arg(valist, char *);
    argv[i] = tmp;
  }
  argv[cpt + 1] = NULL;
  va_end(valist);

  // EXEC
  int res = execv(path, argv);
  checkCond(res == -1, "Error EXECL");

  // free up argv
  free(argv);
  return res;
}

//*****************************************************************************
// PIPE
//*****************************************************************************
int spipe(int pipefd[2])
{
  int resv = pipe(pipefd);
  checkNeg(resv, "Error PIPE");
  return resv;
}

//*****************************************************************************
// SIGNAL
//*****************************************************************************

void ehandler(int signum)
{
}

void ssigaction(int signum, void (*handler)(int signum))
{
  struct sigaction action;
  action.sa_handler = handler;
  ssigfillset(&action.sa_mask);
  action.sa_flags = 0;

  int r = sigaction(signum, &action, NULL);
  checkNeg(r, "Error sigaction");
}

void ssigemptyset(sigset_t *set)
{
  int res = sigemptyset(set);
  checkNeg(res, "sigemptyset");
}

void ssigfillset(sigset_t *set)
{
  int res = sigfillset(set);
  checkNeg(res, "sigfillset");
}

void ssigaddset(sigset_t *set, int signum)
{
  int res = sigaddset(set, signum);
  checkNeg(res, "sigaddset");
}

void ssigdelset(sigset_t *set, int signum)
{
  int res = sigdelset(set, signum);
  checkNeg(res, "sigdelset");
}

void ssigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
  int res = sigprocmask(how, set, oldset);
  checkNeg(res, "sigprocmask");
}

void skill(pid_t pid, int signum)
{
  int res = kill(pid, signum);
  checkNeg(res, "kill");
}

void ssigpending(sigset_t *set)
{
  int ret = sigpending(set);
  checkNeg(ret, "ssigpending");
}

//******************************************************************************
// SHARED MEMORY
//******************************************************************************

int sshmget(key_t key, size_t size, int shmflg)
{
  int shm_id = shmget(key, size, shmflg);
  checkNeg(shm_id, "Error shmget");
  return shm_id;
}

void *sshmat(int shm_id)
{
  void *ptr = shmat(shm_id, NULL, 0);
  checkCond(ptr == (void *)-1, "Error shmat");
  return ptr;
}

void sshmdt(const void *shmaddr)
{
  int res = shmdt(shmaddr);
  checkNeg(res, "Error shmdt");
}

void sshmdelete(int shm_id)
{
  int r = shmctl(shm_id, IPC_RMID, NULL);
  checkNeg(r, "Error shmctl");
}

//******************************************************************************
// SEMAPHORES
//******************************************************************************

int sem_create(key_t key, int nsems, int perm, int val)
{
  int sem_id = semget(key, nsems, IPC_CREAT | perm);
  checkNeg(sem_id, "Error semget in sem_create");

  int i = 0;
  union semun arg;
  arg.val = val;
  while (i != nsems)
  {
    int rv = semctl(sem_id, i, SETVAL, arg);
    checkNeg(rv, "Error semctl in sem_create");
    i++;
  }

  return sem_id;
}

int sem_get(key_t key, int nsems)
{
  int sem_id = semget(key, nsems, 0);
  checkNeg(sem_id, "Error semget in sem_get");
  return sem_id;
}

static void sem_add(int sem_id, int sem_num, int val)
{
  struct sembuf sem;
  sem.sem_num = sem_num;
  sem.sem_op = val;
  sem.sem_flg = 0;

  int rc = semop(sem_id, &sem, 1);
  checkNeg(rc, "Error semop in sem_add");
}

void sem_down(int sem_id, int sem_num)
{
  sem_add(sem_id, sem_num, -1);
}

void sem_down0(int sem_id)
{
  sem_add(sem_id, 0, -1);
}

void sem_up(int sem_id, int sem_num)
{
  sem_add(sem_id, sem_num, 1);
}

void sem_up0(int sem_id)
{
  sem_add(sem_id, 0, 1);
}

void sem_delete(int sem_id)
{
  int rv = semctl(sem_id, 0, IPC_RMID);
  checkNeg(rv, "Error semctl in sem_delete");
}

//******************************************************************************
// SOCKETS
//******************************************************************************

int ssocket()
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  checkNeg(sockfd, "socket client creation error");
  return sockfd;
}

int sconnect(char *serverIP, int serverPort, int sockfd)
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr)); /* en System V */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort);
  inet_aton(serverIP, &addr.sin_addr);
  int ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  checkNeg(ret, "connect client error");
  return ret;
}

int sbind(int port, int sockfd)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  checkNeg(ret, "server bind error");
  return ret;
}

int slisten(int sockfd, int backlog)
{
  int ret = listen(sockfd, backlog);
  checkNeg(ret, "listen error error");
  return ret;
}

int saccept(int sockfd)
{
  int newsockfd = accept(sockfd, NULL, NULL);
  checkNeg(newsockfd, "accept failure");

  return newsockfd;
}

void hostname_to_ip(char *hostname, char *ip)
{
  struct hostent *he = gethostbyname(hostname);
  checkCond(he == NULL, "gethostbyname problem");

  struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
  checkCond(addr_list[0] == NULL, "No address found");

  char *adr = inet_ntoa(*addr_list[0]);
  checkCond(adr == 0, "PRB inet_ntoa");
  strcpy(ip, adr);
}

int spoll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  int ret = poll(fds, nfds, timeout);
  checkNeg(ret, "poll failure");

  return ret;
}
