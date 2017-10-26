import os
import sys
import traceback
import boto3
import logging
from urllib.request import urlopen


controller_base_url = 'https://requestb.in/1mgxkmz1?command='
debug = bool(os.environ.get('DEBUG'))
allowed_commands = ['close', 'open', 'toggle', 'stop']
logging.basicConfig(level=(logging.DEBUG if debug else logging.INFO),
                    format="%(asctime)s | %(levelname)s | %(name)s: %(funcName)s() | %(message)s")


def long_poll_messages():
    sqs = boto3.resource('sqs')
    queue = sqs.get_queue_by_name(QueueName='HomeControlCurtains')
    while True:
        # Process messages by printing out body and optional command name
        for message in queue.receive_messages(MessageAttributeNames=['Command'],
                                              WaitTimeSeconds=20):
            try:
                handle_message(message)
            except Exception as e:
                logging.error('Exception while handling message')
                traceback.print_exc(file=sys.stderr)
            finally:
                message.delete()


def handle_message(message):
    logging.debug('New message')
    if not message.message_attributes:
        logging.warning('Attributes were not set')
        return

    command = message.message_attributes.get('Command').get('StringValue')
    if command not in allowed_commands:
        logging.warning('Command "%s" is not allowed' % command)
        return

    handle_command(command)


def handle_command(command):
    url = '%s%s' % (controller_base_url, command)
    logging.info('Request GET "%s"' % url)
    resp = urlopen(url, method='GET', timeout=2)
    logging.info('Result %s %s' % (resp.status, resp.read()))


if __name__ == '__main__':
    long_poll_messages()
