# CronCppParser
A parser for Cron syntax that follows whatever specified in [CronTabGuru](https://crontab.guru/)
Try to make it easy to use with simple interfaces:
 - Create a Parser to validate the syntax of Cron
 - Get a **Cron Clock Object** to determine the next **time_point** of scheduler specified by given Cron Expression
# Sample
```cpp
    std::string expression = "0 0 1-25/3 1/3 *";
    try
    {
        auto clock = Cron::Parser(expression).createClock();
        //you can also use clock.getNext() to get std::chrono::time_point value
        std::cout << "your next action from now is at: " << clock.getNextCTime() << std::endl;
        std::cout << "Next 10 scheduled Time from now is: \n";
        for(int i = 0; i < 10; ++i)
        {
            std::cout << clock.getNextCTime(false /*true = current moment*/) << "\n";
        }
    }
    catch(const Cron::BadSyntaxException& s)
    {
        std::cout << s << std::endl;
    }
    catch(const Cron::ImpossibleValueException s1)
    {
        std::cout << s1 << std::endl;
    }
```
    **Possible output:**
    your next action from now is at: Mon Apr 01 00:00:00 2019
    Next 10 scheduled Time from now is: 
    Thu Apr 04 00:00:00 2019
    Sun Apr 07 00:00:00 2019
    Wed Apr 10 00:00:00 2019
    Sat Apr 13 00:00:00 2019
    Tue Apr 16 00:00:00 2019
    Fri Apr 19 00:00:00 2019
    Mon Apr 22 00:00:00 2019
    Thu Apr 25 00:00:00 2019
    Mon Jul 01 00:00:00 2019
    Thu Jul 04 00:00:00 2019

# Error report
```cpp
try
    {
        std::string expression = "0 0 35 2 *";
        auto clock = Cron::Parser(expression).createClock();
    }
    catch(const Cron::BadSyntaxException& s)
    {
        std::cout << s << std::endl;
    }
    catch(const Cron::ImpossibleValueException& s1)
    {
        std::cout << s1 << std::endl;
    }
```
    **Bad Syntax at field MDAY[ 35 ]: Invalid field mday with value: 35**

