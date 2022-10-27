# Riddle

This is a walkthrough of the __riddle__ CTF. It consists of 3 tiers and each tier has a number of levels you have to pass.

## Tier 1

### Challenge 0

The first challenge wants us to create a file name **.hello_there** in the same directory as **riddle**. We do this, because strace tells us that **riddle** tries to open a file (with the following system call) which is simply not there.
```c
openat(AT_FDCWD, ".hello_there", O_RDONLY)
```
_Solution_:
```bash
echo "" >> .hello_there
```

### **Challenge 1**

The hint informs us that “The door is left unlocked” and thus we suspect permissions. In addition, _strace_ shows us that the executable tries to open then file created in __Challenge 0__ with _O_WRONLY_ (write only) permissions. The system call succeeds,, so the executable CAN in fact open the file with write permission and this is not what we want. We revoke the write access for the current user and the challenge passes!

_Solution_:
```bash
chmod -w .hello_there
```

### **Challenge 2**


Στο hint αναφέρεται ότι θέλει βοήθεια να προχωρήσει και μαζί με το **SIGCONT** που του ορίζεται καινούριος handler σκεφτόμαστε ότι η λύση είναι μάλλον να σταλθεί ένα **SIGCONT** στην διεργασία, και πράγματι δουλεύει.

Running _strace_ again, we observe that the process defines 2 signal handlers for the signals __SIGARLM__ and __SIGCONT__. It then asks for a __SIGARLM__ to signal it in 10 seconds (using the syscal `alarm(10)` ) and then suspends itself using the syscall `pause()`.
The hint refers that the process needs some help to move on, and _SINGCONT_ gets a new handler (`rt_sigaction(SIGCONT, {sa_handler=0x..., sa_mask=[CONT], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x...}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0` ) and thus we infer that the solution is to send it a __SIGCONT__ .

Solution_:
```bash
pkill -CONT riddle
```
The same can be achieved by suspending the process ( _Ctrl + z_ ) and resume it using the _"fg"_ shell command.

### **Challenge 3**

This challenge encourages us to use _ltrace_, whose output tells us that the process used the function `getenv("ANSWER")`. This function tries to read the environment variable _ANSWER_, which of course has to reason to exist prior to this, and thus the function call returns _"nil"_. We have to give it a value though, so as to pass the challenge. That value is the answer to life, the universe and everything, which of course is __42__!

_Solution_:
```bash
export "ANSWER"=42
```

### __Challenge 4__

> Παρατήρηση: Παρατηρούμε πως το εκτελέσιμο (απο το ltrace) ψάχνει να διαβάσει ένα env var με όνοομα I_NEED_TECH_HINTS και αποτυγχάνει. Αν το δημιουργήσουμε τότε παρατηρούμε πως το εκτελέσιμο πια μας δείχνει έξτρα hints.

Τρέχοντας πάλι ltrace βλέπουμε πως το εκτελέσιμο ψάχνει να ανοίξει ένα αρχείο magic_mirror άρα το δημιουργούμε. Παρόλα αυτά πάλι το εκτελέσιμο δείχνει fail και παρατηρούμε στο ltrace πως διαβάζει και γράφει από το magic_mirror χαρακτήρες. Γράφει πρώτα και διαβάζει μετά. Άρα το offset εντός του file δείχνει ακριβός μετά τον χαρακτήρα που γράφτηκε.

Αυτά μας οδηγούν στο συμπέρασμα πως το riddle θέλει να γράψει έναν τυχαίο χαρακτήρα και να τον διαβάσει. Αυτό γίνεται μέσω pipes, αν στα άκρα διαβάζει και ακούει το ίδιο εκτελέσιμο. Από το tech hint μαθαίνουμε πως υπάρχουν επώνυμα pipes (θα μας βόλευε αφού το εκτελέσιμο ανοίγει ένα αρχείο magic_mirror και στην τελική τα pipes είναι files) και μπορούμε μάλιστα να δημιουργήσουμε τέτοια με την εντολή mkfifo(). Το κάνουμε (με όνομα magic_mirror) και δουλεύει.

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/d8119788-bf5b-49a5-b16d-2d94024b9d12/Untitled.png)

```bash
mkfifo magic_mirror

```

Φσε

Challenge 5

Τρέχοντας το riddle με strace βλέπουμε πως προσπαθεί να διαβάσει από έναν file descriptor με νούμερο 99 και (προφανώς γιατί δεν υπάρχει) δεν τα καταφέρνει. Το tech hint αναφέρει “dup, dup2” το οποίο αν γκουγκλάρουμε είναι 2 system calls που δημιουργόυν καινούριους file descriptor που όμως αναφέρονται στο ίδιο αρχείο με κάποιν ήδη υπάρχον. Συγκεκριμένα η dup2(2) μας επιτρέπει να επιλέξουμε και την τιμή του νέου fd (η dup(2) απλά παίρνει τον επόμενο αύξοντα διαθέσιμο). Άρα με κάποιο τρόπο θα χρησιμοποιήσουμε την dup2(2) για να δημιουργήσουμε το νέο αυτό fd.

Ένας τρόπος να το κάνουμε αυτό είναι να δημιουργήσουμε το file discriptor μόνοι μας και στην συνέχεια να κάνουμε execve(./riddle) και έτσι η riddle θα κληρονομήσει τα fd του python script.

Και πράγματι εκτελόντας το python script βλέπουμε πως το challenge 5 επιτυγχάνει

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/cd8e3102-c529-4f25-ad4b-8569097a557c/Untitled.png)

```python
import os
os.dup2(1,99)
os.execv("./riddle",["riddle"])

```

```bash
python ch5_script.py

```

Challenge 6

Τρέχοντας strace -f στο riddle (ώστε να δούμε και system calls τυχόν παιδιών) παρατηρούμε πως δημιουργεί 2 παιδία τα οποία προσπαθούν να γράψουν σε 2 file descriptors με νούμερα 33 και 34.

Το λύνουμε με τον ακριβώς από πάνω τρόπο και επαναλαμβάνουμε. Ανακαλύπτουμε ότι έχουμε EBADF και για 2 fd 53,54. Τροποποιούμε το script και τα δημιουργούμε. Τώρα το strace δείχνει ότι και το κάθε παιδί γράφει στον πρώτο fd και διαβάζει από τον επόμενο για το ζεύγος 33,34 και το ανάποδο για το ζεύγος 53,54. Επίσης φαίνεται πως εγείρουν και τα 2 ένα SIGALRM. Αν τρέχουμε το script παρατηρούμε πως εκτειπόνεται ένα PING, η εκτέλεση κολλάει και απλά σταματάει μετά από 2 δευτερόλεπτα. Αυτό μας δίνει σοβαρές υποψίες ότι τα παιδιά κάνουν read από κενό αρχείο και lockαρουν στο read μέχρι να τους έρθει το SIGALRM και τερματίσουν. Επίσης ότι το κάθε παιδί γράφει στο ένα από τα fd 34,54 και το άλλο διαβάζει από το προηγούμενο (33, 53) μας κάνει να πιστεύουμε ότι έχουμε να κάνουμε με 2 pipes. Τροποποιούμε το script και έτσι δημιουργόυμε 2 pipes τα οποία και αντιγράφουμε στα θεμιτά fds {33,34 - 53,54} με την χρήση της dup2(2) και στην συνέχεια καλούμε execv για το riddle. Τρέχουμε το scirpt και βλέπουμε πως επιτυγχάνει.

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/d68f31fa-96de-452f-ab4a-fbc56a683e94/Untitled.png)

```python
import os

fds = os.pipe()+os.pipe()

os.dup2(fds[0],33)
os.dup2(fds[1],34)
os.dup2(fds[2],53)
os.dup2(fds[3],54)

os.execv("./riddle",["riddle"])

```

```bash
python ch6_script.py

```

Challenge 7

Τρέχοντας strace στο riddle βλέπουμε πως προσπαθεί να πάρει πληροφορίες (lstat(2)) για 2 αρχεία, το .hello_there και το .hey_there. Το .hey_there δεν υπάρχει άρα το δημιουργούμε.

Πάλι ωστόσο αποτυγχάνει με ένα μύνημα (Oops. 3410258 != 3409984.). Ανεξαρτήτουε εκτέλεσης τα νούμερα παραμένουν ίδια. Τρέχοντας την εντολή _stat .hey_there .hello_there παρατηρούμε πως τα νούμερα αυτά είναι τα inodes των 2 αρχείων. Προφανώς δεν είναι ίδια όπως σωστά λέει το riddle γιατί είναι άλλα αρχεία._

Ο μόνος τρόπος να έχουν ίδιο inode είναι να είναι το ένα hard link του άλλου, κάτι που μας το προδίδει και το tech hint που αναφέρει “hard links” αλλά και η παρόμοια ονομασία των αρχείων. Σβήνουμε το .hey_there και το ξαναδημιουργόυμε σαν hard link του .hello_there, τρέχουμε το riddle και το challenge 7 επιτυγχάνει.

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/e2cfd95d-839a-4219-8f27-191a34b45270/Untitled.png)

```bash
ln .hello_there .hey_there

```

Challenge 8

Τρέχοντας ltrace στο riddle βλέπουμε πως προσπαθεί να ανοίξει ένα αρχείο με όνομα bf00, το οποίο προφανώς και δεν υπάρχει και θα δημιουργήσουμε. Στο ltrace παρατηρούμε πως μέσω του lseek(2) θέτει το offset ίσο με ένα 1GB και προσπαθεί να διαβάσει. Το αρχείο είναι άδειο άρα αυτό δεν γίνεται. Δημιουργούμε random data στο αρχείο με μέγεθος > 1Ghead -c 2G </dev/urandom >bf00

και επαναλαμβάνουμε το ltrace. Ανακαλύπτουμε οτι η ίδια ιστορία γίνεται και για ένα αρχείο bf01, bf02 οπότε βαριόμαστε αυτή την μηχανική και βαρετή (και σπάταλη σε χώρο) διαδικασία (σπάταλη γιατι σε κάθε αρχείο διαβάζει μετά από 1GB οπότε αυτός ο χώρος ότι και να έχει μέσα πάει τσάμπα. Αν ψάξουμε το tech hint “sparse files” βρίσκουμε πως αυτά θα ταίριαζαν ακριβώς για την δουλειά. Άρα αρκεί να βρούμε πως υλοποιούνται σε Unix.

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/80f69e70-0ec9-40ac-bdf8-e185be5cfc6e/Untitled.png)

Θα χρησιμοποιήσουμε εντός ενός shell script την εντολή truncate για να δημιουργήοσουμε αρχεία μηδενικού content μεγέθους 1GB και θα του κάνουμε append στο τέλος 16 “x” χαρακτήρες. Αυτό θα μπει εντός ενός for loop για να δημιουργηθούν 10 τέτοια αρχεία και αν το riddle θέλει παραπάνω απλά θα πειράξουμε το πάνω όριο του for. Το δοκιμάζουμε και βλέπουμε πως 10 αρχεία αρκούν.

```bash
#!/bin/bash
for i in {0..9}
do
    truncate -s 1G bf0$i
    echo "xxxxxxxxxxxxxxxx" >> bf0$i
done
./riddle

```

Challenge 9

Τρέχοντας ένα strace στο riddle βλέπουμε πως δημιουργεί ένα socket τύπου IPv4. Στην συνέχεια προσπαθεί να το συνδέσει στον [localhost](http://localhost) στο πορτ 49842 και αποτυγχάνει με το error “ECONNREFUSED”. Προφανώς το error προκύπτει διότι δεν υπάρχει κανένας στο localhost να ακούει και να δέχεται συνδέσεις σε αυτή την πόρτα. Οπότε και θα δημιουργουσήσουμε έναν απλό listener μέσω του netcat. Σηκώνουμε το listener και τρέχουμε πάλι το riddle το οποίο στέλνει ένα μύνημα τύπου captcha το οποίο και αν απαντήσουμε επιτυγχάνει το level.

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/7a69bcf6-eebe-4680-af4b-eb42c946a494/Untitled.png)

```bash
nc -l localhost 49842

```

Challenge 10

Τρέχοντας strace στο riddle βλέπουμε πως δημιουργεί ένα αρχείο με όνομα “secret_number” και στην συνέχεια καλεί την unlink πάνω του. Το documentation της unlink αναφέρει πως διαγράφει από το file system ένα αρχείο όταν δεν υπάρχει πια κανένας file descriptor να αναφέρεται σε αυτό, άρα εδώ προς το παρόν υπάρχει ακόμα. Στην συνέχεια καλείται η mmap και δημιουργείται ένα memory mapping με διεύθηνση που κανονίζεται από τον kernel, μήκους 4KB, shared (μπορουν να την έχουν και άλλα processes) το οποίο αρχικοποιείται με τα περιεχόμενα του file descriptor 4, δηλαδή του “secret_number”. Έπειτα ο fd που αντιστοιχίζεται στο αρχείο αυτό κλείνει και τώρα διαγράφεται οριστικά από το file system. Αυτή την στιγμή το περιεχόμενο του βρίσκεται μόνο στα περιεχόμενα εκείνου του mapping και από εκεί θα πρέπει να βρούμε έναν τρόπο να το διαβάσουμε.

Θα ανοίξουμε ένα δεύτερο shell και με την εντολή ps -a θα βρούμε το pid του riddle. Θα πλοηγηθούμε στο φάκελο /proc/$riddle_pid/map_files και θα κάνουμε cat το αρχείο που αντιστοιχίζεται στην διεύθηνση που γυρνάει η mmap εντός του strace (Είναι και κόκκινο γιατί δείχνει mapping προς διεγραμμένο αρχείο). Θα χρειαστεί να κάνουμε sudo cat δίοτι δεν έχουμε (προφανώς και θεμιτά) σαν απλός χρήστης δικαιώματα να διαβάζουμε τον χώρο μνήμης κάθε διεργασίας.

```bash
1st command ->
ps -a | grep "riddle"

2nd command ->
cd /proc/$riddle_pid/map_files

3rd command ->
sudo cat $memory_map_address

```

Challenge 11

Λύνεται ακριβώς με τον ίδιο από πάνω τρόπο.

Challenge 12

Τρέχοντας strace στο riddle βλέπουμε πως δημιουργεί ένα tmp αρχείο και το αντιστοιχεί με τον παραπάνω τρόπο (των 2 προηγούμενων challenge) με ένας εύρος εικονικών διευθύνσεων. Περιμένει να βρει έναν χαρακτήρα στην θέση 0x6f εντός του αρχείου. Αυτό που θα κάνουμε έιναι για να μην μας ενοχλεί το ρολόι, θα αναστείλουμε με ctrl z το riddle. Στην συνέχεια θα βρούμε το pid του riddle και θα κάνουμε cd στον φάκελο του εντός του /proc/. Από εκεί θα μπούμε στον φάκελο map_files και θα ανοίξουμε με sudo δικαιώματα το αρχείο στο vim. Θα αντικαταστήσουμε τον 112ο χαρακτήρα με τον ζητούμενο και θα αποθηκεύσουμε τις αλλαγές. Αν τώρα επαναφέρουμε το riddle με την εντολή “fg” θα δούμε πως επιτυγχάνει

```bash
1st step
./riddle

2nd step
ctrl+z

3rd step
ps -a | grep "riddle"

4rth step
cd /proc/$pid/map_files όπου $pid το pid που βρήκαμε στο step 3

5th step
ls -la

6th
sudo vim $address_range
όπου address_range το νούμερο που αντιστοιχίζεται στο tmp αρχείο στην έξοδο του ls

7th
αντικατάσταση του 112ου χαρακτήρα με τον εκάστοτε που φαίνεται στην έξοδο του riddle

```

Challenge 13

Τρέχοντας strace στο riddle παρατηρούμε πως προσπαθεί να ανοίξει το .hello_there με read write permissions που δεν έχει και πετάει EACCES διότι το αρχείο δεν έχει write δικαιώματα τα οποία θα του δώσουμε με την chmod.

```bash
chmod +w .hello_there

```

Στην συνέχεια αυξάνεται το μέγεθος του αρχείου στα 32ΚΒ και γίνεται map στην μνήμη. Στην συνέχεια το μέγεθος μειώνεται στα 16ΚΒ και το riddle περιμένει input. Ότι και να του δώσουμε πετάει SIGBUS διότι προσπαθεί να διαβάσει μνήμη η οποία τυπικά δεν έχει κάτι μέσα (το τέλος του mapped αρχείου είναι αρκετά πριν την mapped διεύθυνση, γνωστό bug ως bus error →paging error.

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/72c9eb31-f48a-4a92-82ad-1202d9460947/Untitled.png)

Για να λύσουμε το πρόβλημα θα χρειαστεί να κάνουμε truncate το αρχείο πάλι στο αρχικό του μέγεθος των 32ΚΒ. Αυτό θα το κάνομε μέσω της εντολής truncate. Την εκτελούμε, πατάμε Enter στο riddle ώστε να προχωρήσει και επιτυγχάνει το challenge 13.

```bash
truncate --size=32K .hello_there

```

Challenge 14

Τρέχοντας strace στο riddle βλέπουμε πως για να επιτύχει το challenge θέλει να εκτελεστεί με συγκεκριμένο pid. Θα τροποποιήσουμε το αρχείο ns_last_pid στο φάκελο /proc/sys/kernel/.

Βάζουμε το επιθυμητό pid-1 μέσα στο αρχείο και εκτελούμε ακριβώς μετά το riddle.
