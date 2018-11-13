function v = helics_flag_interruptible()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 38);
  end
  v = vInitialized;
end
