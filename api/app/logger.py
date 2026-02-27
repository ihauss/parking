import logging
import sys


def setup_logger(name: str = "smart_parking", level=logging.INFO) -> logging.Logger:
    logger = logging.getLogger(name)

    # Évite les handlers dupliqués au reload
    if logger.handlers:
        return logger

    logger.setLevel(level)

    handler = logging.StreamHandler(sys.stdout)
    handler.setLevel(level)

    formatter = logging.Formatter(
        "[%(asctime)s] [%(levelname)s] %(name)s - %(message)s"
    )
    handler.setFormatter(formatter)

    logger.addHandler(handler)
    logger.propagate = False

    return logger