from ctypes import *
import os


class CoverageLL:
    module = None
    context = None

    def __init__(self, _id):
        self.module = cdll.LoadLibrary('libcov.so')
        self.module.get_arr.restype = POINTER(c_uint)
        assert os.getpid() == self.module.pid(), "Not on same pid weirdly"
        self.context = self.module.new_context(_id)

    def cov_initialize(self):
        return self.module.cov_initialize(self.context)

    def cov_finish_initialization(self):
        return self.module.cov_finish_initialization(self.context)

    def cov_shutdown(self):
        return self.module.cov_shutdown(self.context)

    def cov_evaluate_crash(self):
        return self.module.cov_evaluate_crash(self.context)

    def cov_clear_bitmap(self):
        return self.module.cov_clear_bitmap(self.context)

    def cov_evaluate(self):
        edges = self.module.new_edge_set()
        result = self.module.cov_evaluate(self.context, edges)
        return result

    def cov_compare_equal(self):
        return self.module.cov_compare_equal(self.context)

    def found_edge(self):
        return self.module.found_edge(self.context)

    def __del__(self):
        self.module.free_context(self.context)


class Coverage(CoverageLL):

    def __init__(self, _id):
        super().__init__(_id)
        pid = os.getpid()
        assert self.cov_initialize() == 0, "Failed to initialize"
        os.environ["SHM_ID"] = f"shm_id_{pid}_{_id}"

    def post_boot(self):
        self.cov_finish_initialization()

    def pre_execute(self):
        self.cov_clear_bitmap()

    def __del__(self):
        # unlink the shared memory regions on shutdown
        self.cov_shutdown()
