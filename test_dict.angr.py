import angr
import claripy
import sys

init_dict_addr = 0x4012B0 
file_name = 'list.txt'
file_size = 100

def main():
    proj = angr.Project("./GS")

    first_state = proj.factory.blank_state(addr=init_dict_addr)
    
    symbolic_file_backing_memory = angr.storage.memory_mixins.DefaultMemory()
    symbolic_file_backing_memory.set_state(first_state)

    _dict = claripy.BVS('dict', file_size * 8)
    _dict_file = angr.storage.SimFile(file_name, writable=False, content=_dict, size=file_size)
    symbolic_filesystem = {
        file_name : _dict_file
    }
    first_state.posix.fs = symbolic_filesystem
    w = angr.SIM_PROCEDURES['libc']['fopen'](project=proj)

    first_state.memory.store(0x69000000, file_name.encode('ascii')+b"\x00")
    first_state.memory.store(0x69000010, b"r\x00")
    f = w.execute(first_state, arguments=[0x69000000, 0x69000010]).ret_expr
    import IPython; IPython.embed()
    #first_state.regs.rsi =
    #first_state.regs.rdi =

    #sim = simulation = proj.factory.simgr(initial_state)

    return 0

if __name__ == "__main__":
    sys.exit(main())

