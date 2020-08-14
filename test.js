const chai = require('chai')
const expect = chai.expect

const nodePython = require('./build/Release/PyNode')
const { promisify } = require('util')

if (process.platform === 'linux') nodePython.dlOpen('libpython3.6m.so')
nodePython.startInterpreter()
nodePython.appendSysPath('./test_files')
nodePython.openFile('tools')

const call = promisify(nodePython.call)

describe('nodePython', () => {
  describe('#eval', () => {
    it('successful eval should return 0', () => {
      let response = nodePython.eval('import sys')
      expect(response).to.equal(0)
    })

    it('failed eval should return -1', () => {
      let response = nodePython.eval('import randommodulethatshouldnotexist11')
      expect(response).to.equal(-1)
    })
  })

  describe('#call', () => {
    it('should fail if the last parameter is not a function', () => {
      try {
        nodePython.call('return_immediate', 2)
      } catch (err) {
        expect(err.message).to.equal("Last argument to 'call' must be a function")
      }
    })

    it('should return the stack trace', done => {
      call('causes_runtime_error')
        .then(result => console.log('should not see this : ', result))
        .catch(err => {
          expect(err.message.includes('causes_runtime_error')).to.equal(true)
          expect(err.message.includes('name \'secon\' is not defined')).to.equal(true)
          done()
        })
    })

    it('should return the time series data', done => {
      call('time_series_data')
        .then(result => {
          expect(typeof result[0][0]).to.equal('string')
          expect(typeof result[0][1]).to.equal('number')
          done()
        })
    })

    it('should throw an exception with the wrong number of arguments', done => {
      call('return_immediate', 9, 9, 9)
        .catch(e => {
          expect(e.message).to.equal("The function 'return_immediate' has 1 arguments, 3 were passed")
          done()
        })
    })

    it('should return the correct value when passing Int32', done => {
      call('return_immediate', 2)
        .then(result => {
          expect(result).to.equal(2)
          done()
        })
    })

    it('should return the correct value when passing Float', done => {
      call('return_immediate', 3.14)
        .then(result => {
          expect(result).to.equal(3.14)
          done()
        })
    })

    it('should return the correct value when passing String', done => {
      call('return_immediate', 'the string')
        .then(result => {
          expect(result).to.equal('the string')
          done()
        })
    })

    it('should return the correct value when passing bool', done => {
      call('return_immediate', true)
        .then(result => {
          expect(result).to.equal(true)
          done()
        })
    })

    it('should return the correct value when passing bool', done => {
      call('return_immediate', false)
        .then(result => {
          expect(result).to.equal(false)
          done()
        })
    })

    it('should return null when python returns None', done => {
      call('return_none')
        .then(result => {
          expect(result).to.equal(null)
          done()
        })
    })

    it.skip('should return the correct value when passing Date', done => {
      let d = new Date()
      call('return_immediate', d)
        .then(result => {
          expect(result).to.equal(d)
          done()
        })
    })

    describe('arrays', () => {
      it('should return the correct value when passing an empty array', done => {
        call('return_immediate', [])
          .then(result => {
            expect(result).to.deep.equal([])
            done()
          })
      })

      it('should return the correct value when passing an array of ints', done => {
        call('return_immediate', [1, 2, 3])
          .then(result => {
            expect(result).to.deep.equal([1, 2, 3])
            done()
          })
      })

      it('should return the correct value when passing an array of strings', done => {
        call('return_immediate', ['a', 'b', 'c'])
          .then(result => {
            expect(result).to.deep.equal(['a', 'b', 'c'])
            done()
          })
      })

      it('should return the correct value when passing an array of mixed types', done => {
        call('return_immediate', ['a', 1, 6.7777])
          .then(result => {
            expect(result).to.deep.equal(['a', 1, 6.7777])
            done()
          })
      })

      it('should return the correct value when passing a nested array', done => {
        let x = [
          [1, 2, 3],
          ['a', 'b', 'c']
        ]
        call('return_immediate', x)
          .then(result => {
            expect(result).to.deep.equal(x)
            done()
          })
      })

      it('should return the correct value when passing arrays with objects', done => {
        let x = [
          { array: [1, 2, 3] },
          { string: 'ok', float: 8281.111 }
        ]
        call('return_immediate', x)
          .then(result => {
            expect(result).to.deep.equal(x)
            done()
          })
      })

      it('should return sum of numeric array input', done => {
        call('sum_items', [1, 2, 3])
          .then(result => {
            expect(result).to.equals(6)
            done()
          })
      })
    })

    describe('tuples', () => {
      it.skip('should return an array when a tuple is returned from python', done => {
        call('return_tuple')
          .then(result => {
            expect(result).to.deep.equal([1, 2, 3])
            done()
          })
      })
    })

    describe('objects / dicts', () => {
      it('should return an object from a python dict', done => {
        call('return_dict')
          .then(result => {
            expect(result).to.deep.equal({'size': 71589, 'min': -99.6654762642, 'max': 879.08351843})
            done()
          })
      })

      it('should return the correct value when passing an object', done => {
        call('return_immediate', {})
          .then(result => {
            expect(result).to.deep.equal({})
            done()
          })
      })

      it('should return the correct value when passing an object', done => {
        call('return_immediate', { hey: 'guys' })
          .then(result => {
            expect(result).to.deep.equal({ hey: 'guys' })
            done()
          })
      })

      it('should return the correct value when passing an object', done => {
        call('return_immediate', { hey: 'guys', other: 1 })
          .then(result => {
            expect(result).to.deep.equal({ hey: 'guys', other: 1 })
            done()
          })
      })

      it('should return the correct value when passing an object', done => {
        let crazyObj = {
          hey: [1, 2, 3],
          no: "yes",
          x: {
            stuff: 'no',
            things: ['a', 1, 2.33],
            bro: {
              x: 1.233
            }
          }
        }
        call('return_immediate', crazyObj)
          .then(result => {
            expect(result).to.deep.equal(crazyObj)
            done()
          })
      })

      it('should merge two dicts', done => {
        let x = {
          hey: 'guys'
        }
        let y = {
          other: 1
        }
        call('merge_two_dicts', x, y)
          .then(result => {
            expect(result).to.deep.equal({ hey: 'guys', other: 1 })
            done()
          })
      })
    })
  })

  // describe('stopInterpreter', () => {
  //   it('should stop the interpreter', () => {
  //     nodePython.stopInterpreter()
  //   })
  // })
})
