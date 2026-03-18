import logging
import sys


def setup_logger(
    name: str = "smart_parking",
    level: int = logging.INFO
) -> logging.Logger:
    """
    Configure and return a logger instance.

    This function ensures:
    - No duplicate handlers (safe for hot reload environments)
    - Consistent log formatting across the application
    - Logs are written to stdout (container-friendly)

    Args:
        name (str): Logger name
        level (int): Logging level (e.g., logging.INFO, logging.DEBUG)

    Returns:
        logging.Logger: Configured logger instance
    """
    logger = logging.getLogger(name)

    # Avoid adding multiple handlers (e.g., during reload)
    if logger.handlers:
        return logger

    logger.setLevel(level)

    # Stream handler (stdout for Docker / cloud environments)
    handler = logging.StreamHandler(sys.stdout)
    handler.setLevel(level)

    # Standard log format
    formatter = logging.Formatter(
        "[%(asctime)s] [%(levelname)s] [%(name)s] %(message)s"
    )
    handler.setFormatter(formatter)

    logger.addHandler(handler)

    # Prevent logs from being propagated to the root logger
    logger.propagate = False

    return logger