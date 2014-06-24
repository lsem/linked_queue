linked_queue
============

Demonstration of technique of maintaining a queue inside storage without havingh separated queue object. May be useful for cases when you want to insert elements into storage as quickly as possible and do not want to creating new object and place them in queue. 




       
        First            Last
            |             |
            v             |
    ------------     -----|------     
    |        o-|---> |    |-->o-|----> NULL
    |          |     |          |     
    ------------     ------------     



  Queuing next record for processing:
              
            1) *Last = &NEW;  

               First             Last
                 |                |
                 V                |
            ------------     -----|------     
            |        n-|---> |    |-->n |
            |          |     |        | |     
            ------------     ---------|--     
                                      |
                                      V
                                 ------------
                                 |   NEW  n-|--> NULL
                                 |          |
                                 ------------

            2) Last = &NEW->n;
               First             Last
                 |                 |
                 V                 ---------
            ------------     ------------  |   
            |        n-|---> |        n |  |
            |          |     |        | |  |  
            ------------     ---------|--  |  
                                      |    |
                                      V    |
                                  ---------V--
                                  |   NEW  n-|--> NULL
                                  |          |
                                  ------------

        ------------------------------------
        | Single item queue case:
        ------------------------------------

               First (T*)       Last (T**)
                 |                |
                 |  ---------------
                 V  |
            --------V----    
            |       o--|---> NULL
            |          |     
            ------------     


