function v = halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 9);
  end
  v = vInitialized;
end
