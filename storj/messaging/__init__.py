import importlib
import pkgutil

from storj.messaging.messaging import Messaging


def get_messaging_class():
    count = 0
    cls = None
    import storj.messaging
    for (importer, name, isPkg) in pkgutil.iter_modules(
                                    path=storj.messaging.__path__):
        if isPkg:
            m = importlib.import_module('storj.messaging.' + name)
            if hasattr(m, 'messaging_class'):
                cls = getattr(m, 'messaging_class')
                if issubclass(cls, Messaging):
                    count += 1

    if count == 0:
        return None
    if count > 1:
        raise IOError('There are more than one messaging classes')

    return cls
