# NestReader

## Log reader for [LogNest](https://github.com/LeaoMartelo2/lognest)

This is a very simple general use Linux CLI Tool for reading log files, made specially for 
[lognest](https://github.com/LeaoMartelo2/lognest)


## Usage

```bash
./nestreader [options] <filename>
```
### The option flags are:

#### Filtering options:
<pre>
            -d, --debug         Print DEBUG messages    
            -t, --trace         Print TRACE/LOG messages
            -w, --warn          Print WARN messages     
            -e, --error         Print ERROR messages    
            -a, --all           Prints all messages     
</pre>

#### Other options:
<pre>
            -n, --no-color      Disables terminal scape-code colors
            -h, --help          Display help                  
</pre>

The program will listen to updates to the file specified, and pring whatever new lines (passing by the filter) Toolthe standart output, by default with terminal escape colors.


## Building

Only supports Linux.

`./build.sh`
 
## ETC

The source code is very hackable and self explorable, you can modify it for your needs very easily.
