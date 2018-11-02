function v = helics_flag_interruptible()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856893);
  end
  v = vInitialized;
end
