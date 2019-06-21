const expect = require('chai').expect
const nodePython = require('./build/Release/PyNode')

describe('nodePython', () => {
  describe('#call', () => {

    it('should throw an exception with the wrong number of arguments', () => {
      let fn = () => { nodePython.call('return_immediate', 9, 9, 9) }
      expect(fn).to.throw(Error, "The function 'return_immediate' has 1 arguments, 3 were passed")
    })

    it('should return the correct value when passing Int32', () => {
      expect(nodePython.call('return_immediate', 2)).to.equal(2)
    })

    it('should return the correct value when passing Float', () => {
      expect(nodePython.call('return_immediate', 3.14)).to.equal(3.14)
    })

    it('should return the correct value when passing String', () => {
      expect(nodePython.call('return_immediate', 'the string')).to.equal('the string')
    })

    it('should return the correct value when passing bool', () => {
      expect(nodePython.call('return_immediate', true)).to.equal(true)
    })

    it('should return the correct value when passing bool', () => {
      expect(nodePython.call('return_immediate', false)).to.equal(false)
    })

    it.skip('should return the correct value when passing Date', () => {
      let d = new Date()
      expect(nodePython.call('return_immediate', d)).to.equal(d)
    })

    describe('arrays', () => {
      it('should return the correct value when passing an empty array', () => {
        expect(nodePython.call('return_immediate', [])).to.deep.equal([])
      })

      it('should return the correct value when passing an array of ints', () => {
        expect(nodePython.call('return_immediate', [1, 2, 3])).to.deep.equal([1, 2, 3])
      })

      it('should return the correct value when passing an array of strings', () => {
        expect(nodePython.call('return_immediate', ['a', 'b', 'c'])).to.deep.equal(['a', 'b', 'c'])
      })

      it('should return the correct value when passing an array of mixed types', () => {
        expect(nodePython.call('return_immediate', ['a', 1, 6.7777])).to.deep.equal(['a', 1, 6.7777])
      })

      it('should return the correct value when passing a nested array', () => {
        let x = [
          [1, 2, 3],
          ['a', 'b', 'c']
        ]
        expect(nodePython.call('return_immediate', x)).to.deep.equal(x)
      })

      it.skip('should return the correct value when passing arrays with objects', () => {
        let x = [
          { array: [1, 2, 3] },
          { string: 'ok', float: 8281.111 }
        ]
        expect(nodePython.call('return_immediate', x)).to.deep.equal(x)
      })
    })

    describe('objects / dicts', () => {
      it('should return the correct value when passing an object', () => {
        expect(nodePython.call('return_immediate', {})).to.deep.equal({})
      })

      it('should return the correct value when passing an object', () => {
        expect(nodePython.call('return_immediate', { hey: 'guys' })).to.deep.equal({ hey: 'guys' })
      })

      it('should return the correct value when passing an object', () => {
        expect(nodePython.call('return_immediate', { hey: 'guys', other: 1 })).to.deep.equal({ hey: 'guys', other: 1 })
      })

      it('should return the correct value when passing an object', () => {
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
        expect(nodePython.call('return_immediate', crazyObj)).to.deep.equal(crazyObj)
      })
    })
  })
})
