from unittest.mock import Mock, patch
import time

from network.transport.tcp_transport import TcpTransport


class TestTcpTransport:
    """Test suite for TcpTransport class."""

    def test_initialisation(self):
        """Test TcpTransport initialises with correct file descriptor."""
        transport = TcpTransport(socket_fd=5)

        assert transport.fd == 5
        assert transport.running is False
        assert transport.reader_thread is None

    @patch("network.transport.tcp_transport.os.write")
    def test_send_success(self, mock_write):
        """Test successful data sending."""
        transport = TcpTransport(socket_fd=5)
        test_data = "test message"

        transport.send(test_data)

        mock_write.assert_called_once_with(5, b"test message")

    def test_send_on_closed_socket(self):
        """Test sending data on closed socket logs error."""
        transport = TcpTransport(socket_fd=-1)

        with patch.object(transport.logger_, "error") as mock_error:
            transport.send("test")
            mock_error.assert_called_once_with("Cannot send: socket closed")

    @patch("network.transport.tcp_transport.os.write")
    def test_send_with_os_error(self, mock_write):
        """Test send handles OSError gracefully."""
        mock_write.side_effect = OSError("Connection broken")
        transport = TcpTransport(socket_fd=5)

        with patch.object(transport.logger_, "error") as mock_error:
            transport.send("test")
            assert mock_error.called
            assert "Send error:" in mock_error.call_args[0][0]

    @patch("network.transport.tcp_transport.threading.Thread")
    @patch("network.transport.tcp_transport.os.read")
    def test_start_creates_reader_thread(self, mock_read, mock_thread):
        """Test start() creates and starts reader thread."""
        # Mock read to return data once, then empty (connection closed)
        mock_read.side_effect = [b"test data", b""]

        transport = TcpTransport(socket_fd=5)
        callback = Mock()

        transport.start(callback)

        # Verify thread was created and started
        mock_thread.assert_called_once()
        assert transport.running is True

        # Get the thread target function
        thread_kwargs = mock_thread.call_args[1]
        assert thread_kwargs["daemon"] is True

    def test_start_on_invalid_socket(self):
        """Test start() on invalid socket logs error."""
        transport = TcpTransport(socket_fd=-1)
        callback = Mock()

        with patch.object(transport.logger_, "error") as mock_error:
            transport.start(callback)
            mock_error.assert_called_once_with("Cannot start: not connected")
            assert transport.running is False


class TestTcpTransportReaderLoop:
    """Test suite for TcpTransport reader loop functionality."""

    @patch("network.transport.tcp_transport.os.read")
    def test_reader_loop_receives_data(self, mock_read):
        """Test reader loop receives and processes data."""
        # Simulate receiving data then connection close
        mock_read.side_effect = [b"message 1", b"message 2", b""]  # Connection closed

        transport = TcpTransport(socket_fd=5)
        callback = Mock()

        transport.start(callback)

        # Wait for thread to process
        time.sleep(0.1)

        # Verify callback was called with decoded messages
        assert callback.call_count == 2
        callback.assert_any_call("message 1")
        callback.assert_any_call("message 2")

    @patch("network.transport.tcp_transport.os.read")
    def test_reader_loop_handles_empty_data(self, mock_read):
        """Test reader loop handles connection closure (empty data)."""
        mock_read.return_value = b""

        transport = TcpTransport(socket_fd=5)
        callback = Mock()

        with patch.object(transport.logger_, "info") as mock_info:
            transport.start(callback)
            time.sleep(0.1)

            # Verify connection close was logged
            mock_info.assert_called_with("Server closed connection")
            assert transport.running is False

    @patch("network.transport.tcp_transport.os.read")
    def test_reader_loop_handles_os_error(self, mock_read):
        """Test reader loop handles OSError."""
        mock_read.side_effect = OSError("Network error")

        transport = TcpTransport(socket_fd=5)
        callback = Mock()

        with patch.object(transport.logger_, "error") as mock_error:
            transport.start(callback)
            time.sleep(0.1)

            # Verify error was logged
            assert mock_error.called
            assert "Read error:" in mock_error.call_args[0][0]
            assert transport.running is False

    @patch("network.transport.tcp_transport.os.read")
    def test_reader_loop_handles_decode_error(self, mock_read):
        """Test reader loop handles decoding errors."""
        # Invalid UTF-8 sequence
        mock_read.side_effect = [b"\xff\xfe", b""]

        transport = TcpTransport(socket_fd=5)
        callback = Mock()

        with patch.object(transport.logger_, "info"):
            transport.start(callback)
            time.sleep(0.1)

            # Verify error handling
            assert transport.running is False
