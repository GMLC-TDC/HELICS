function v = iteration_halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 14);
  end
  v = vInitialized;
end
