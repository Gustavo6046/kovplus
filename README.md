# kovplus

kovplus is a small library intended as a proof-of-concept for a new
simplistic algorithm to sentence-generating chatbots, an iteration
on the old Weighed Monte-Carlo Markov Chain approach that your neighbour's
backyard IRC bot probably uses.

Rather than simply storing probability weights for the next word, kovplus
stores each next-word occurence with a window of expected surrounding words;
then when a new word is to be generated, it assesses which would be most
likely to appear given the words before that.

For example:

```
  - Current word: growth

  - Known responses:

(1) ...soil  is     important for     the  [growth of]   plants...
(2) ...sadly common to        undergo some [growth that] might...
(3) ...in    the    winter    we      see  [growth of]   frost...

  - New response so far:

...it    is     important that    the  [growth] {???}

  - Assessments:

   'it'        -> (1) not 'soil',      (2) not 'sadly',   (3) not 'in'
   'is'        -> (1) yes 'is',        (2) not 'common',  (3) not 'the'
   'important' -> (1) yes 'important'. (2) not 'to',      (3) not 'winter'
   'that'      -> (1) not 'for',       (2) not 'undergo', (3) not 'we'
   'the'       -> (1) yes 'the',       (2) not 'some',    (3) not 'see'
   
   Tally:         (1) 3,               (2) 0,             (3) 0

  - The winner is (1) 'growth of'

  - New response:

	...it is important that the growth of...
    
```

Repeat 'ad nauseum' - that is, until we are satisfied.

We do not simply pick the max tally results. We use a weighed random choice, although only
with non-zero results. Otherwise, everything would be too samey, wouldn't it?
