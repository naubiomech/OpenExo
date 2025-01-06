import tkinter as tk

class CustomKeyboard(tk.Frame):
    def __init__(self, parent, target_widget, on_submit=None):
        super().__init__(parent)
        self.target_widget = target_widget  # The input widget the keyboard interacts with
        self.on_submit = on_submit  # Callback for when the user submits the value

        # Define button layout
        keys = [
            ['7', '8', '9'],
            ['4', '5', '6'],
            ['1', '2', '3'],
            ['0', '.', 'Clear'],
            ['Submit']
        ]

        # Set default geometry for the keyboard window
        parent.geometry("300x400")  # Original size: 300x400 pixels
        
        # Label to show current value
        self.value_label = tk.Label(self, text=self.target_widget.get(), font=("Arial", 16))
        self.value_label.pack(side=tk.TOP, fill=tk.BOTH, pady=10)  # Add some padding for better spacing

        # Create buttons
        for row in keys:
            row_frame = tk.Frame(self)
            row_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
            for key in row:
                btn = tk.Button(
                    row_frame, text=key, font=("Arial", 16),
                    command=lambda k=key: self.handle_key(k)
                )
                btn.pack(side=tk.LEFT, expand=True, fill=tk.BOTH)

    def handle_key(self, key):
        if key == "Clear":
            self.target_widget.delete(0, tk.END)  # Clear the widget
        elif key == "Submit":
            if self.on_submit:
                self.on_submit(self.target_widget.get())  # Trigger callback with the value
        else:
            self.target_widget.insert(tk.END, key)  # Add key value to the widget

        # Update the label to show the current value in the target widget
        if self.value_label.winfo_exists():
            self.value_label.config(text=self.target_widget.get())

    def set_target(self, target_widget):
        """Update the target widget the keyboard interacts with."""
        self.target_widget = target_widget
        if self.value_label.winfo_exists():
            self.value_label.config(text=self.target_widget.get())
