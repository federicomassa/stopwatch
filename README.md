Easy stopwatch to measure time intervals between different code portions and evaluate mean and std deviation.

## Usage

```
Tazio::Stopwatch sw;

// Optional 
sw.set_output_file("/home/user/log.txt");

for (...iterations...) {
    sw.init(); // Fix initial time
    
    // ... do stuff

    sw.partial("label1");

    // ... do stuff

    sw.partial("label2");

    // ....

    sw.end();
    }
```
