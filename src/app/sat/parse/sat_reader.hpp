
#ifndef MALLOB_SAT_READER_H
#define MALLOB_SAT_READER_H

#include <string>
#include <vector>
#include <memory>
#include <stdio.h> 
#include "util/assert.hpp"

#include "data/job_description.hpp"

#include <iostream>

class SatReader {

private:
    std::string _filename;
    bool _raw_content_mode;

    // Content mode: ASCII
    int _sign = 1;
	bool _comment = false;
	bool _began_num = false;
    bool _assumption = false;
	int _num = 0;
	int _max_var = 0;

    // Content mode: RAW
    bool _traversing_clauses = true;
    bool _traversing_assumptions = false;
    bool _empty_clause = true;

    bool _valid_input = false;

public:
    SatReader(const std::string& filename) : _filename(filename) {}
    bool read(JobDescription& desc);

    inline void processInt(int x, JobDescription& desc) {
        
        //std::cout << x << std::endl;

        if (_valid_input) {
            // Already WAS valid input: additional numbers will make it invalid!
            _valid_input = false;
            return;
        }

        if (_empty_clause && x == 0) {
            // Received an "empty clause" (zero without a preceding non-zero literal)
            if (_traversing_clauses) {
                // switch to assumptions
                _traversing_clauses = false;
                _traversing_assumptions = true;
            } else if (_traversing_assumptions) {
                // End of assumptions: done.
                _valid_input = true;
            }
            return;
        }
        
        if (_traversing_clauses) desc.addPermanentData(x);
        else desc.addTransientData(x);

        _max_var = std::max(_max_var, std::abs(x));
        _empty_clause = _traversing_assumptions || (x == 0);
    }

    inline void process(char c, JobDescription& desc) {

        if (_comment && c != '\n') return;

        signed char uc = *((signed char*) &c);
        switch (uc) {
        case '\n':
        case '\r':
        case EOF:
            _comment = false;
            if (_began_num) {
                if (_num != 0) {
                    _valid_input = false;
                    return;
                }
                if (!_assumption) desc.addPermanentData(0);
                _began_num = false;
            }
            _assumption = false;
            break;
        case 'p':
        case 'c':
            _comment = true;
            break;
        case 'a':
            _assumption = true;
            break;
        case ' ':
            if (_began_num) {
                _max_var = std::max(_max_var, _num);
                if (!_assumption) {
                    desc.addPermanentData(_sign * _num);
                } else if (_num != 0) {
                    desc.addTransientData(_sign * _num);
                }
                _num = 0;
                _began_num = false;
            }
            _sign = 1;
            break;
        case '-':
            _sign = -1;
            _began_num = true;
            break;
        default:
            // Add digit to current number
            _num = _num*10 + (c-'0');
            _began_num = true;
            break;
        }
    }

    bool isValidInput() const {
        return _valid_input;
    }
};

#endif