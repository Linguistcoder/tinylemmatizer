// tinylemmatizer.c : Defines the entry point for the application.
//

#include <stdio.h>
#include <string.h>
//#include <malloc.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "tinylemmatizer.h"

static char* result = 0;
static char* buf = 0;
static long buflen = 0;

typedef struct LemmaRule
    {
    const char* Lem;
    } LemmaRule;

typedef struct lemmaCandidate
    {
    const char* L;
    bool ruleHasPrefix;
    } lemmaCandidate;

typedef struct startEnd
    {
    const char* s;
    const char* e;
    } startEnd;

typedef unsigned char typetype;

static LemmaRule* newStyleLemmatizeV3
(const char* word
    , const char* wordend
    , const char* buf
    , const char* maxpos
    , lemmaCandidate* parentcandidate
    , LemmaRule* lemmas
);


static const char* samestart(const char** fields, const char* s, const char* we)
    {
    const char* f = fields[0];
    const char* e = fields[1] - 1;
    while ((f < e) && (s < we) && (*f == *s))
        {
        ++f;
        ++s;
        }
    // On success: return pointer to first unparsed character
    // On failure: return 0
    return f == e ? s : 0;
    }

static const char* sameend(const char** fields, const char* s, const char* wordend)
    {
    const char* f = fields[2];
    const char* e = fields[3] - 1;
    const char* S = wordend - (e - f);
    if (S >= s)
        {
        s = S;
        while (f < e && *f == *S)
            {
            ++f;
            ++S;
            }
        }
    // On success: return pointer to successor of last unparsed character
    // On failure: return 0
    return f == e ? s : 0;
    }

static bool substr(const char** fields, int k, const char* w, const char* wend, startEnd* vars, int vindex)
    {
    if (w == wend)
        return false;
    const char* f = fields[k];
    const char* e = fields[k + 1] - 1;
    const char* p = w;
    assert(f != e);
    const char* ff;
    const char* pp;
    do
        {
        while ((p < wend) && (*p != *f))
            {
            ++p;
            }
        if (p == wend)
            return false;
        pp = ++p;
        ff = f + 1;
        while (ff < e)
            {
            if (pp == wend)
                return false;
            if (*pp != *ff)
                break;
            ++pp;
            ++ff;
            }
        } while (ff != e);
        vars[vindex].e = p - 1;
        vars[vindex + 1].s = pp;
        return true;
    }

static char* rewrite(const char** pword, const char** pwordend, const char* p)
    {
    startEnd vars[20];
    const char* fields[44]; // 44 = (2*20 + 3) + 1
    // The even numbered fields contain patterns
    // The odd numbered fields contain replacements
    // The first two fields (0,1) refer to the prefix
    // The third and fourth (2,3) fields refer to the suffix
    // The remaining fields (4,5,..,..) refer to infixes, from left to right
    // input =fields[0]+vars[0]+fields[4]+vars[1]+fields[6]+vars[2]+...+fields[2*n+2]+vars[n]+...+fields[2]
    // output=fields[1]+vars[0]+fields[5]+vars[1]+fields[7]+vars[2]+...+fields[2*n+3]+vars[n]+...+fields[3]
    // where 'vars[k]' is the value caught by the k-th wildcard, k >= 0
    const char* wend = *pwordend;
    fields[0] = p;
    int findex = 1;
    while (*p != '\n')
        {
        if (*p == '\t')
            fields[findex++] = ++p;
        else
            ++p;
        }
    fields[findex] = ++p;
    // fields[findex] points to character after \n. 
    // When 1 is subtracted, it points to the character following the last replacement.
    // p is now within 3 bytes from the first Record of the subtree
           // check Lpat
    vars[0].s = samestart(fields, *pword, wend);
    if (vars[0].s)
        {
        // Lpat succeeded
        vars[0].e = wend;
        char* destination = NULL;
        int printed = 0;
        if (findex > 2) // there is more than just a prefix
            {
            const char* newend = sameend(fields, vars[0].s, wend);
            if (newend)
                wend = newend;
            else
                return 0; //suffix didn't match

            int k;
            const char* w = vars[0].s;
            int vindex = 0;
            for (k = 4; k < findex; k += 2)
                {
                if (!substr(fields, k, w, wend, vars, vindex))
                    break;
                ++vindex;
                w = vars[vindex].s;
                }
            if (k < findex)
                return 0;

            vars[vindex].e = newend;
            //                     length of prefix       length of first unmatched         length of suffix
            ptrdiff_t resultlength = (fields[2] - fields[1] - 1) + (vars[0].e - vars[0].s) + (fields[4] - fields[3] - 1);/*20120709 int -> ptrdiff_t*/
            int m;
            for (m = 1; 2 * m + 3 < findex; ++m)
                {
                int M = 2 * m + 3;
                //                    length of infix       length of unmatched after infix
                resultlength += (fields[M + 1] - fields[M] - 1) + (vars[m].e - vars[m].s);
                }
            //++news;
            destination = (char*)malloc(resultlength + 1);
            printed = sprintf(destination, "%.*s%.*s", (int)(fields[2] - fields[1] - 1), fields[1], (int)(vars[0].e - vars[0].s), vars[0].s);
            for (m = 1; 2 * m + 3 < findex; ++m)
                {
                int M = 2 * m + 3;
                printed += sprintf(destination + printed, "%.*s%.*s", (int)(fields[M + 1] - fields[M] - 1), fields[M], (int)(vars[m].e - vars[m].s), vars[m].s);
                }
            printed += sprintf(destination + printed, "%.*s", (int)(fields[4] - fields[3] - 1), fields[3]);
            *pword = vars[0].s;
            *pwordend = newend;
            }
        else if (vars[0].e == vars[0].s) // whole-word match: everything matched by "prefix"
            {
            //++news;
            destination = (char*)malloc((fields[2] - fields[1] - 1) + 1);
            printed = sprintf(destination, "%.*s", (int)(fields[2] - fields[1] - 1), fields[1]);
            }
        else
            return 0; // something unmatched

        return destination;
        }
    else
        {
        // Lpat failed
        return 0; // prefix failed
        }
    }

static LemmaRule* addLemma(LemmaRule* lemmas, lemmaCandidate* lemma)
    {
    if (lemma->L)
        {
        if (lemmas)
            {
            int i;
            for (i = 0; lemmas[i].Lem; ++i)
                {
                if (!strcmp(lemmas[i].Lem, lemma->L))
                    {
                    return lemmas;
                    }
                }
            LemmaRule* nlemmas = (LemmaRule*)calloc(i + 2, sizeof(LemmaRule));
            for (i = 0; lemmas[i].Lem; ++i)
                {
                nlemmas[i] = lemmas[i];
                }
            free(lemmas);
            lemmas = nlemmas;
            lemmas[i].Lem = lemma->L;
            lemma->L = 0;
            lemmas[++i].Lem = 0;
            }
        else
            {
            lemmas = (LemmaRule*)calloc(2, sizeof(LemmaRule));
            lemmas[1].Lem = 0;
            lemmas[0].Lem = lemma->L;
            lemma->L = 0;
            }
        }
    return lemmas;
    }

static LemmaRule* chainV3
    ( const char* word
    , const char* wordend
    , const char* buf
    , const char* maxpos
    , lemmaCandidate* parentcandidate
    , LemmaRule* lemmas
    )
    {
    for (int next = *(int*)buf
        ;
        ; buf += next, next = *(int*)buf
        )
        {
        assert((next & 3) == 0);
        assert(next == -4 || next == 0 || next == 4 || next >= 12);
        if (next == -4 || next == 4)
            {
            // add parent candidate to lemmas.
            lemmas = addLemma(lemmas, parentcandidate);
            }
        else
            {
            LemmaRule* temp = newStyleLemmatizeV3(word, wordend, buf + sizeof(int), next > 0 ? buf + next : maxpos, parentcandidate, lemmas);
            if (temp)
                {
                lemmas = temp;
                }
            else
                {
                lemmas = addLemma(lemmas, parentcandidate);
                }
            }
        if (next <= 0)
            break;
        }
    return lemmas;
    }

static LemmaRule* newStyleLemmatizeV3
    ( const char* word
    , const char* wordend
    , const char* buf
    , const char* maxpos
    , lemmaCandidate* parentcandidate
    , LemmaRule* lemmas
    )
    {
    if (maxpos <= buf)
        return 0;
    const char* cword = word;
    const char* cwordend = wordend;
    int pos = 0;
    pos = *(int*)buf;
    const char* until;
    LemmaRule* Result;
    assert((pos & 3) == 0);
    assert(pos >= 0);
    if (pos == 0)
        until = maxpos;
    else
        until = buf + pos;
    typetype type;
    const char* p = buf + sizeof(int);
    type = *(typetype*)p;
    /*
    buf+4
    first bit  0: Fail branch is unambiguous, buf points to tree. (A)
    first bit  1: Fail branch is ambiguous, buf points to chain. (B)
    second bit 0: Success branch is unambiguous, buf+8 points to tree (C)
    second bit 1: Success branch is ambiguous, buf+8 points to chain (D)
    */
    if (type < 4)
        {
        ++p;
        }
    else
        {
        type = 0; // no ambiguity
        }
    lemmaCandidate candidate;
    if (*p && *p != '\t')
        {
        candidate.ruleHasPrefix = true;
        }
    else
        candidate.ruleHasPrefix = parentcandidate ? parentcandidate->ruleHasPrefix : false;
    candidate.L = rewrite(&cword, &cwordend, p);
    p = strchr(p, '\n');
    ptrdiff_t off = p - buf;
    off += sizeof(int);
    off /= sizeof(int);
    off *= sizeof(int);
    p = buf + off;

    if (candidate.L)
        {
        lemmaCandidate* defaultCandidate = candidate.L[0] ? &candidate : parentcandidate;
        /* 20150806 A match resulting in a zero-length candidate is valid for
        descending, but if all descendants fail, the candidate is overruled by
        an ancestor that is not zero-length. (The top rule just copies the
        input, so there is a always a non-zero length ancestor.) */
        switch (type)
            {
                case 0:
                case 1:
                    {
                    /* Unambiguous children. If no child succeeds, take the
                    candidate, otherwise take the succeeding child's result. */
                    LemmaRule* childcandidates = newStyleLemmatizeV3(cword, cwordend, p, until, defaultCandidate, lemmas);
                    Result = childcandidates ? childcandidates : addLemma(lemmas, defaultCandidate);
                    free((void*)candidate.L);
                    candidate.L = 0;
                    break;
                    }
                case 2:
                case 3:
                    {
                    /* Ambiguous children. If no child succeeds, take the
                    candidate, otherwise take the succeeding children's result
                    Some child may in fact refer to its parent, which is our
                    current candidate. We pass the candidate so it can be put
                    in the right position in the sequence of answers. */
                    LemmaRule* childcandidates = chainV3(cword, cwordend, p, until, defaultCandidate, lemmas);
                    Result = childcandidates ? childcandidates : addLemma(lemmas, defaultCandidate);
                    free((void*)candidate.L);
                    candidate.L = 0;
                    break;
                    }
                default:
                    Result = lemmas;
            }
        }
    else
        {
        switch (type)
            {
                case 0:
                case 2:
                    {
                    /* Unambiguous siblings. If no sibling succeeds, take the
                    parent's candidate. */
                    LemmaRule* childcandidates = newStyleLemmatizeV3(word, wordend, until, maxpos, parentcandidate, lemmas);
                    Result = childcandidates != 0 ? childcandidates : addLemma(lemmas, parentcandidate);
                    break;
                    }
                case 1:
                case 3:
                    {
                    /* Ambiguous siblings. If a sibling fails, the parent's
                    candidate is taken. */
                    LemmaRule* childcandidates = chainV3(word, wordend, until, maxpos, parentcandidate, lemmas);
                    Result = childcandidates != 0 ? childcandidates : addLemma(lemmas, parentcandidate);
                    break;
                    }
                default:
                    Result = lemmas;
            }
        }
    return Result;
    }


static char* concat(LemmaRule* L)
    {
    if (L)
        {
        size_t lngth = 0;
        int i;
        for (i = 0; L[i].Lem; ++i)
            {
            lngth += strlen(L[i].Lem) + 1;
            }
        ++lngth;
        char* ret = (char*)malloc(lngth);
        ret[0] = 0;
        for (i = 0; L[i].Lem; ++i)
            {
            strcat(ret, L[i].Lem);
            free((void*)L[i].Lem);
            L[i].Lem = 0;
            strcat(ret, " ");
            }
        free(L);
        ret[lngth - 1] = 0;
        return ret;
        }
    else
        return 0;
    }

static LemmaRule* pruneEquals(LemmaRule* L)
    {
    for (int i = 0; L[i].Lem; ++i)
        {
        for (int j = i + 1; L[j].Lem; ++j)
            {
            if (!strcmp(L[i].Lem, L[j].Lem))
                {
                free((void*)L[j].Lem);
                for (int k = j; L[k].Lem; ++k)
                    {
                    L[k].Lem = L[k + 1].Lem;
                    }
                }
            }
        }
    return L;
    }

static const char* apply(const char* word
    , const char* buf
    , const char* maxpos
)
    {
    size_t len = strlen(word);
    free(result);
    result = 0;
    LemmaRule* lemmas = 0;
    result = concat(pruneEquals(newStyleLemmatizeV3(word, word + len, buf, maxpos, 0, lemmas)));
    return result;
    }


static const char* applyRules(const char* word)
    {
    if (buf)
        return apply(word, buf, buf + buflen);
    return 0;
    }

int readRules(const char* filename)
    {
    FILE* flexrulefile = fopen(filename, "rb");
    if (flexrulefile)
        {
        fseek(flexrulefile, 0, SEEK_END);
        buflen = ftell(flexrulefile);
        fseek(flexrulefile, sizeof(int), SEEK_SET);
        buflen -= sizeof(int);
        buf = (char*)malloc(buflen + 1);
        if (buf && buflen > 0)
            {
            if (fread(buf, 1, buflen, flexrulefile) != (size_t)buflen)
                {
                fclose(flexrulefile);
                return 2;
                }
            buf[buflen] = '\0';
            }
        fclose(flexrulefile);
        return 0;
        }
    return 1;
    }

void startProc(const char* flexrulesFileName, int* err)
    {
    *err = readRules(flexrulesFileName);
    }

void stringEval(const char* s, const char** out, int* err)
    {
    *out = applyRules(s);
    *err = 0;
    }

void endProc(void)
    {
    free(result);
    free(buf);
    result = 0;
    buf = 0;
    buflen = 0;
    }

int main(int argc, char ** argv)
    {
    const char* res = 0;
    int err = 0;
    const char* filename = 0;
    const char* fullform = 0;
    if (argc > 1)
        filename = argv[1];
    else
        filename = "flexrules";
    if (argc > 2)
        fullform = argv[2];
    startProc(filename,&err);
    switch (err)
        {
            case 0:
                if (fullform)
                    {
                    stringEval(fullform, &res, &err);
                    printf("%s\n",res);
                    }
                else
                    {
                    while (1)
                        {
                        char string[20];
                        fgets(string, sizeof(string), stdin);
                        while (1)
                            {
                            long L = (long)strlen(string) - 1;
                            if (L < 0)
                                break;
                            int c = string[L];
                            if (c == '\r' || c == '\n' || c == ' ')
                                string[L] = '\0';
                            else
                                break;
                            }
                        if (string[0] == '\0')
                            break;
                        stringEval(string, &res, &err);
                        printf("result:%s\n", res);
                        }
                    }
                endProc();
                break;
            case 1:
                printf("Cannot open file %s\n", filename);
                break;
            case 2:
                printf("Something went wromg when reading %s\n", filename);
                break;
            default:
                printf("Unknown error\n");
        }
    return 0;
    }
