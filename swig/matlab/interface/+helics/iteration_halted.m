function v = iteration_halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 16);
  end
  v = vInitialized;
end
